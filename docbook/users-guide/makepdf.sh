#!/bin/sh
#
# Generate the ROOT User's Guide in PDF format.

docbookdirs="/usr/share/xml/docbook/stylesheet/docbook-xsl \
             /sw/share/xml/xsl/docbook-xsl"

fopjars="/usr/share/java/fop.jar \
         /sw/share/java/fop/fop.jar"

docbook=
for d in $docbookdirs; do
   if [ -d $d ]; then
      docbook=$d
   fi
done

if [ -z $docbook ]; then
   echo "No docbook installation found"
   exit 1
fi

fopjar=
for f in $fopjars; do
   if [ -f $f ]; then
      fopjar=$f
   fi
done

if [ -z $fopjar ]; then
   echo "No fop.jar file found"
   exit 1
fi

# for more printed output options see:
# http://xml.web.cern.ch/XML/www.sagehill.net/xml/docbookxsl/PrintOutput.html

xsltproc --xinclude --output ROOTUsersGuide.fo \
   --stringparam paper.type A4 \
   $docbook/fo/docbook.xsl \
   ROOTUsersGuide.xml

java -Xmx1024m -jar $fopjar ROOTUsersGuide.fo ROOTUsersGuide.pdf
rm ROOTUsersGuide.fo

exit 0
