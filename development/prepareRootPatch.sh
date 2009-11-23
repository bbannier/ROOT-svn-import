#!/usr/local/bin/zsh

### Step 0: DQ
# 1) first, make sure the HEAD of TMVA-SVN is what we want in the ROOT release !
# 2) also make sure that the patches in ROOT SVN are applied to TMVA
# so if it is Friday night, think again and wait until Saturday morning !

### Step 1: Checkout
# this is a temporary directory in which we will create all necessary files for Rene
cd $HOME; mkdir -p TMVAintoROOT; cd TMVAintoROOT 
#
# checkout the head of root
svn co http://root.cern.ch/svn/root/trunk root
#
# checkout the head of TMVA
svn co https://tmva.svn.sourceforge.net/svnroot/tmva/trunk/TMVA


### Step 2: Create the patch
# now the files from TMVA are copied over two ROOT/tmva (src/inc/test dirs)
./TMVA/development/makeROOTpackage.sh ./rootdev
. ./TMVA/development/createRootPatch.sh



### Step 3: apply and compile the patch
# make sure it works
. ./TMVA/development/applyPatch.sh
cd root
./configure
make all-tmva
cd ..
