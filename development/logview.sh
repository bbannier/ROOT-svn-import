#!/bin/sh
if [ $# == 0 ] ; then
    echo "Usage $0 <file>."
    echo "Tool to cleanup log files created from TMVA output."
    echo "Result will be output on stdout."
    exit 1
fi
#
# The first part of the sed removes all lines beginning with '---' and
# ending with a 0xD (^M). This removes the progressbars.
# The second part removes all remaining escape sequences: (esc)[(any 0-9 or ;)m
#
if [ -r $1 ] ; then
    sed 's/^---.*\x0D//g' $1 | sed 's/\o033\[[0-9;]*m//g'
else
    echo "File not found or not readable: $1"
    exit 1
fi

exit 0

