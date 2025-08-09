function list_open_files()
{
  local -a allies
  allpids=$(set -o pipefail; pstree $1 -Tpa | awk 'BEGIN {FS=","} {print $2}' | awk '{print $1}')
  if [ $? != 0 ];then
    >&2 echo $MY_NAME $(date): Failed to read pstree of $1
    exit 1
  fi
  local PID
  local f
  local wrrd
  local fb
  local flag
  local openfile
  for PID in ${allpids[@]} ; do
    >&2 echo $MY_NAME $(date): DEBUG PID is $PID
    for f in /proc/${PID}/fd/* ;do
      if [[ "$f" == "/proc/${PID}/fd/*" ]]; then
        >&2 echo  $MY_NAME $(date): DEBUG No files opened by ${PID}
        break
      fi
      openfile=$(set -o pipefail; readlink "${f}" | awk '{print $1}')
      #if [ $? != 0 ]; then
      #  >&2 echo $MY_NAME $(date): Failed to readlink ${f} of ${PID}.
      #  exit 1
      #fi
      if [[ ! -z ${openfile} && -f ${openfile} ]]; then
        fb=$(basename $f)
        flag=$(set -o pipefail; cat /proc/${PID}/fdinfo/${fb} | awk '/flags:/{print $2}')
        if [ $? != 0 ]; then
          >&2 echo $MY_NAME $(date): Failed read fdinfo of ${fb} of ${PID}.
          exit 1
        fi
        if [[ $check_wrrd == 1 ]]; then
          wrrd=$((flag&3))
          >&2 echo  $MY_NAME $(date): DEBUG $fb ${openfile} : WRRD is $wrrd FLAG is $flag
          if [[ $wrrd != 0 ]]; then
            echo ${openfile}
          fi
        else
          echo ${openfile}
        fi
      fi
    done
  done
  >&2 echo  $MY_NAME $(date): DEBUG Done.
}

