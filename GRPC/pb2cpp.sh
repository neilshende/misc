#!/bin/bash
tmp=./tmp_intermidiate_script.sh
cat $1 | awk -f proto.awk | tr -d "()" | sed 's/\[/(/g' | sed 's/\]/)/g' >$tmp
cat async_copy_script.sh >>$tmp
chmod a+x $tmp
$($tmp > $2)
