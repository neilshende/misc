#!/bin/bash
# use it like this:
# mvmcli snapshot create -p $p $(gen_savedir $p)

# save all dirs of all open files
# FIXME handle if one dir under another, then only save top dir. haha.
function gen_opendir() 
{

  PID=$1
  u=$((for f in /proc/${PID}/fd/* ;do if [ -f $(readlink $f) ]; then echo $(dirname $(readlink $f)), ; fi;done;) | sort | uniq )
  echo -n --savedir {; for x in ${u[@]} ;do echo -n $x; done; echo -n }
}

# save all open files
# FIXME handle duplicates
function gen_savefile()
{
  PID=$1
  echo -n --savedir { 
  for f in /proc/${PID}/fd/* ;do
    if [ -f $(readlink $f) ]; then
      echo -n $(readlink $f),
    fi
  done
  echo -n }
}

#
# save all open files. If file opened more than once, don't add again to tar ball.
# 
#   use:
#   mvmcli snapshot create -p $p $(gen_openfile $p)
#
function gen_openfile() 
{
  PID=$1
  u=$((for f in /proc/${PID}/fd/* ;do if [ -f $(readlink $f) ]; then echo $(readlink $f), ; fi;done;) | sort | uniq )
  echo -n --savedir {; for x in ${u[@]} ;do echo -n $x; done; echo -n }
}

