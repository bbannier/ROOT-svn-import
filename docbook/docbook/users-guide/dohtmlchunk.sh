set -x

OXYGENHOME=/Users/couet/Desktop/oxygen

xsltproc --xinclude --output ROOTUsersGuide.html \
$OXYGENHOME/frameworks/docbook/xsl/html/chunk.xsl \
ROOTUsersGuide.xml
