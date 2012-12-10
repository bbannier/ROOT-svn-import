target_root=/home/ibucur/root
source_root=/home/ibucur/rootdev/trunk3

cd $target_root
svn update
cd $OLDPWD

for source in $@
do
	source=$(readlink -f $source)
	target=${source#$source_root}
	target=$target_root$target
	cp $source $(dirname $target)
	cd $target_root
	svn diff ${target#$target_root/} > "$ROOTDEV/$(basename $target).diff"
	cd $OLDPWD
	svn revert $target
done


