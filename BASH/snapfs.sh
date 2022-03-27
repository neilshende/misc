#!/bin/bash
POSTDUMP="post-dump"
PRERESTORE="pre-restore"
function dir_slash_ending()
{
   local d=$1
   if [ -d $d ]; then if [[ ! $d =~ /$ ]]; then d=${d}/; fi; fi
   echo $d
}

function is_absolute()
{
   if [[ $1 =~ ^/ ]]; then true; else false; fi
}

function list_open_files_for_write()
{
  local -a PIDS=$(set -o pipefail; pstree $1 -Tpa | awk 'BEGIN {FS=","} {print $2}' | awk '{print $1}')
  if [ $? != 0 ];then
     >&2 echo $MY_NAME $(date): Failed read pstree of $1
     exit 1
  fi
  for PID in ${PIDS[@]} ; do
    >&2 echo PID is $PID
    for f in /proc/${PID}/fd/* ;do
      local openfile=$(set -o pipefail; readlink $f | awk '{print $1}')
      if [ $? != 0 ];then
         >&2 echo $MY_NAME $(date): Failed read open file of $1 .
         exit 1
      fi
      if [ -f $(openfile) ]; then
        local fb=$(basename $f)
        local flag=$(set -o pipefail; cat /proc/${PID}/fdinfo/${fb} | awk '/flags:/{print $2}')
        if [ $? != 0 ];then
          >&2 echo $MY_NAME $(date): Failed read fdinfo of ${PID} .
          exit 1
        fi
        local wrrd=$((flag&3))
          >&2 echo  $MY_NAME $(date): DEBUG $(readlink $f) : WRRD is $wrrd FLAG is $flag
        if [[ $wrrd != 0 ]]; then
          echo $(readlink $f)
        fi
      fi
    done
  done
}

MY_NAME=$(basename "$0")
case "$CRTOOLS_SCRIPT_ACTION" in
    $POSTDUMP )
        if [ "$#" -lt 2 ]; then
            echo "$MY_NAME $(date): ERROR! Missing argument."
            exit 1
        fi
        pid=$1
        shift 1
        save_list=("$@")
        declare -a olist
        declare -a olist_filt
        olist=$(list_open_files_for_write $pid)

        o2list=$(for x in ${olist[@]} ; do echo $x; done | sort -u)
        olist_filt=$(
          for o in ${o2list} ; do
             cont=0
             for sns in "${save_list[@]}" ; do
                if ! is_absolute $sns
                then
                   echo "$MY_NAME $(date): ERROR! [$sns] is not absolute path."
                   exit 1
                fi
                if [ -d $sns ]; then
                   ss=$(dir_slash_ending ${sns})
                   if [[ $o =~ $ss ]]; then
                      cont=1
                      >&2 echo $MY_NAME $(date): DEBUG $o removed found parent dir $ss
                      break
                   fi
                elif [ -f $sns ]; then
                  if [[ "$sns" == "$o" ]]; then
                     cont=1
                     >&2 echo $MY_NAME $(date): DEBUG $o removed found matching file $sns
                     break
                  fi
                fi
             done
             if [[ $cont == 1 ]]; then continue; fi
             echo $o
           done
        )
        declare -a uniq_list
        uniq_list=$(
        for elem in "${save_list[@]}" "${olist_filt[@]}"
        do
           echo $elem
        done | sort -u
        )
        if echo tar -cf ./appdir/backup.tar ${uniq_list[@]}
        then
          echo "$MY_NAME $(date): tar failed."
          exit 1
        fi
        
esac
exit 0

