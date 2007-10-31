#! /bin/sh

CVS2CL=build/unix/cvs2cl.pl

echo ""
echo "Generating README/ChangeLog from CVS logs..."
echo ""

# Generate ChangeLog from version v5-12-00 till now
$SVN2CL -i -f README/ChangeLog -r HEAD:15807

rm -f README/ChangeLog.bak

exit 0
