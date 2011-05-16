set -x
 
OXYGENHOME=/Users/couet/Desktop/oxygen

xsltproc --xinclude --output ROOTUsersGuide.fo \
$OXYGENHOME/frameworks/docbook/xsl/fo/docbook.xsl \
ROOTUsersGuide.xml

java -jar $OXYGENHOME/lib/fop.jar ROOTUsersGuide.fo ROOTUsersGuide.pdf
rm ROOTUsersGuide.fo
