cd $ROOTSYS
rev="`svn update | tail -1 | sed 's/[^0-9]//g'`"
svn merge ^/trunk
svn commit -m "merged with trunk - revision $rev"
make -j 4

