cp $ROOTEST/stressRooStats* ~/root/test

cd ~/root
svn update

svn diff . > patch.diff

svn revert .


