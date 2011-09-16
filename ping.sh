#!/bin/sh

# Example of usage of xrdping in scripts. Accepted arguments are the same as for 'xrdping'.
# USe 'xrdping -h' to find out the available options and syntax.

./xrdping $@ &
wait $!

echo "rc: $?"
 