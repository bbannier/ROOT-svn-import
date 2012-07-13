#!/bin/bash

if [ $# -lt 2 ] 
then
	echo "Not enough arguments"
	exit 1
fi

rta=0; cta=0
for (( c=0; c<$2; c++ )) 
do
	read rt ct <<<$(./$1 | grep RooBernstein | awk '{print $5 " " $10}')
    rta=$(echo "$rta + $rt" | bc)
    cta=$(echo "$cta + $ct" | bc)
    echo $rt " " $ct
done

rta=$(echo "scale=6; $rta / $2" | bc)
cta=$(echo "scale=6; $cta / $2" | bc)

echo "Average real time: " $rta
echo "Average CPU time: " $cta


