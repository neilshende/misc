#!/bin/bash

function cmd() {
 if ! fuser $1 >/dev/null 2>&1; then
    #echo $1 not open
    /bin/rm -f $1
 fi
 }

export -f cmd

find . -name .nfs* -exec bash -c 'cmd "$0"' {} \;

