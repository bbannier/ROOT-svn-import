set -x
xsltproc --xinclude --output ROOTUsersGuide.fo \
/Users/couet/Desktop/oxygen/frameworks/docbook/xsl/fo/docbook.xsl \
ROOTUsersGuide.xml
java -jar  /Users/couet/Desktop/oxygen/lib/fop.jar ROOTUsersGuide.fo ROOTUsersGuide.pdf
rm ROOTUsersGuide.fo