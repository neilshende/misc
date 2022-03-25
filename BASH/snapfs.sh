#!/bin/bash
POSTDUMP="post-dump"
PRERESTORE="pre-restore"
function dirslash()
{
   local d=$1
   if [ -d $d ]; then if [[ ! $d =~ /$ ]]; then d=${d}/; fi; fi
   echo $d
}
function open_files_for_write()
{
  local PID=$1
  for f in /proc/${PID}/fd/* ;do
    if [ -f $(readlink $f | awk '{print $1}') ]; then
      local fb=$(basename $f)
      local flag=$(cat /proc/${PID}/fdinfo/${fb} | awk '/flags:/{print $2}')
      local wrrd=$((flag&2))
          >&2 echo WRRD is $wrrd FLAG is $flag
      if [[ $wrrd != 0 ]]; then
          echo $(readlink $f)
      fi
    fi
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
        olist=$(open_files_for_write $pid)
        olist_filt=$(
          for o in "${olist[@]}" ; do
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

