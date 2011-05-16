#!/bin/sh

docbookdirs="/usr/share/xml/docbook/stylesheet/docbook-xsl \
             /sw/share/xml/xsl/docbook-xsl"

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

xsltproc --xinclude --output ROOTUsersGuide.html \
   $docbook/html/docbook.xsl \
   ROOTUsersGuide.xml
