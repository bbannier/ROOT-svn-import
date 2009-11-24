
cd ./root

diff --brief -r -x 'doc' -x '.svn' -x 'Module.mk' -u tmva ../rootdev/tmva >! ../brieflisting.txt

diff  -r -x 'doc' -x '.svn' -x 'Module.mk' -I '$Id' -u tmva ../rootdev/tmva >! ../tmva.patch

cd ..

newfilesname=
if [[ `grep 'Only in ../rootdev/tmva/' brieflisting.txt | wc -l` > 0 ]]; then
    newfilesname=Newfiles.tgz
    tar -czf $newfilesname `grep 'Only in ../rootdev/tmva/' brieflisting.txt | sed  's#Only in ../#./#' | sed 's#: #/#'`
fi

removedfilesname=
if [[ `grep 'Only in tmva/' brieflisting.txt | wc -l` > 0 ]]; then
    removedfilesname=RemovedFileList.txt
    grep 'Only in tmva/' brieflisting.txt | sed  's#Only in #rm ./#' | sed 's#: #/#' >! $removedfilesname
fi

rm -r brieflisting.txt

tar --remove-files -cf  tmvaPatches.tar tmva.patch $newfilesname $removedfilesname

