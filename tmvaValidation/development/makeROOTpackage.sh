#!/bin/sh

SRC=`pwd`"/TMVA"
mkdir -p $1; cd $1
DEST=`pwd`"/tmva"


# from now on we work only with absolute paths LPWD and DEST

echo " take TMVA package from $LPWD and put it to $DEST"


## header files
mkdir -p $DEST/inc
cp $SRC/src/*.h $DEST/inc/
rm -f $DEST/inc/TMVA_Dict.h


## src files
mkdir -p $DEST/src
cp $SRC/src/*.cxx $DEST/src/


## test macros, scrits, and images
mkdir -p $DEST/test
# everything from the macros
cp $SRC/macros/*.C $DEST/test/.
cp $SRC/macros/README $DEST/test/.
# the python example
cp $SRC/python/TMVAClassification.py $DEST/test/.
# and the images
cp $SRC/macros/*.gif $DEST/test/.
cp $SRC/macros/*.png $DEST/test/.

cd -
