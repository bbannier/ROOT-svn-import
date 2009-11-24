
tar -xf tmvaPatches.tar
cd ./root
rm -rf tmva.bu
cp -r tmva tmva.bu
patch -p0 < ../tmva.patch
if [[ -f ../Newfiles.tgz ]]; then
    tar --strip-path 2 -xzf ../Newfiles.tgz 
fi
if [[ -f ../RemovedFileList.txt ]]; then
    . ../RemovedFileList.txt
fi
cd ..
rm -f tmva.patch Newfiles.tgz RemovedFileList.txt
