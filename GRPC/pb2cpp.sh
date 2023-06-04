#!/bin/bash
cat $1 |awk -f proto.awk | tr -d "()" | sed 's/\[/(/g' | sed 's/\]/)/g' >$2
source $2
cat async_copy_script.sh >>$2
chmod a+x $2
./$2 > $3
