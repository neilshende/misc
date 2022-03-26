#!/bin/bash
POSTDUMP="post-dump"
PRERESTORE="pre-restore"
function dirslash()
{
   local d=$1
   if [ -d $d ]; then if [[ ! $d =~ /$ ]]; then d=${d}/; fi; fi
   echo $d
}

function list_open_files_for_write()
{
  local -a PIDS=$(set -o pipefail; pstree $1 -Tpa | awk 'BEGIN {FS=","} {print $2}' | awk '{print $1}')
  if [ $? != 0 ];then
     >&2 echo $MY_NAME Failed read pstree of $1 .
     exit 1
  fi
  for PID in ${PIDS[@]} ; do
    >&2 echo PID is $PID
    for f in /proc/${PID}/fd/* ;do
      local openfile=$(set -o pipefail; readlink $f | awk '{print $1}')
      if [ $? != 0 ];then
         >&2 echo $MY_NAME Failed read open file of $1 .
         exit 1
      fi
      if [ -f $(openfile) ]; then
        local fb=$(basename $f)
        local flag=$(set -o pipefail; cat /proc/${PID}/fdinfo/${fb} | awk '/flags:/{print $2}')
        if [ $? != 0 ];then
          >&2 echo $MY_NAME Failed read fdinfo of ${PID} .
          exit 1
        fi
        local wrrd=$((flag&3))
          >&2 echo  $(readlink $f) : WRRD is $wrrd FLAG is $flag
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
                if [ -d $sns ]; then
                   ss=$(dirslash ${sns})
                   if [[ $o =~ $ss ]]; then
                      cont=1
                      >&2 echo $o removed found parent dir $ss
                      break
                   fi
                elif [ -f $sns ]; then
                  if [[ "$sns" == "$o" ]]; then
                     cont=1
                     >&2 echo $o removed found matching file $sns
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
        echo DONE ${uniq_list[@]}
        echo OLIST_FILT ${olist_filt[@]}
esac
exit 0

