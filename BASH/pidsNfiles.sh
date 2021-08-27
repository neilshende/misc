#!/bin/bash
# hints
#  $$ is current process
#  $! is pid of child process just created
#  sleep 5000 &
#  printchildren $$
#  sleep 5000 < /tmp/a > /tmp/b 2> /tmp/c &
#  printopenfiles $!
#  useful builtins: dirname basename

function printopenfiles()
{
  PID=$1
  u=$( (for f in /proc/${PID}/fd/* ;do if [ -f $(readlink $f | awk '{print $1}') ]; then echo $(readlink $f); fi;done;) | sort | uniq )
  for x in ${u[@]} ;do echo $x; done
}

function printchildren()
{
  ps -o pid --no-headers --ppid $1
}
