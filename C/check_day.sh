#!/bin/bash
if [ ! -z "$OVERRIDE" ]; then
    echo $OVERRIDE
    exit
fi
day=$(date +%d)
if [ $((day % 2)) -eq 0 ]; then
    echo "EVEN"
else
    echo "ODD"
fi
