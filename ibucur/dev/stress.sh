cd $ROOSTATSTEST

make stressRooStats

if [ $? -ne 0 ] ; then
	exit 1
fi

./stressRooStats -w $1

if [ $? -ne 0 ] ; then
	exit 1
fi

./stressRooStats -f stressRooStats_v534_ref.root $1


