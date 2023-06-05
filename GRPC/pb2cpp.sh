#!/bin/bash
tmp=./tmp_intermediate_script.sh
cat $1 | sed 's/(/ ( /g' | sed 's/)/ ) /g' | awk -f proto.awk > $tmp
cat async_copy_script.sh >>$tmp
chmod a+x $tmp
$($tmp > $2)
