#!/bin/bash

let i=0
rm -f load.in run.in start.in
mkdir -p files

# CINT switches to ".Command"

echo ".h" > start.in
echo ".h" > load.in
echo ".h" > run.in

runwhat="cint cling"
if [ "x$1" = "xcint" -o "x$1" = "xcling" ]; then
    runwhat=$1
    shift
fi

let num=10
if [ "x$1" != "x-f" -a "x$1" != "x" ]; then
    let num=$1
    shift
fi
while [ $i -lt $num ]; do
    name=s$i
    cat skeleton.C | sed 's,%FUNCNAME%,'$name',' > files/$name.C
    echo ".L " files/$name.C >> load.in
    echo ".X " files/$name.C >> run.in
    let i++
done

for job in start load run; do
    file=$job.in
    echo ".X mem.C" >> $file
    echo '.q' >> $file
    echo '.q' >> $file
done

[ "x$1" = "x-f" ] && exit 0

echo Files prepared, now starting

TCM=`ls /usr/lib/*tcmalloc.so* /usr/local/lib/*tcmalloc.so* 2> /dev/null | head -n 1`
if [ "x$TCM" = "x" ]; then
    echo Cannot find libtcmalloc.so
    exit 1
fi

for what in $runwhat; do
    echo -n Running "$what: "
    file=${what}.txt
    echo '===' $what '===' > $file

    for job in load; do # start load run
        echo -n $job"."
        echo -n $job 'time: ' >> $file
        time ($what < $job.in > /dev/null 2>&1 ) 2> time.txt
        cat time.txt | awk '/^r|u|s/ { type=substr($0,1,4); sub(/[[:space:]]/,"",type); sub(/^[[:alpha:][:space:]]+/,""); gsub(/m|\.|s/," "); split($0, mst," "); printf "%s=%d.%ds ", type, mst[1]*60+mst[2], mst[3]; if (type == "sys") print "";}' >> $file
        rm time.txt
        echo -n 'Memory after '$job' (RSS,VSZ [kB]) according to ps: ' >> $file
        cat mem.txt >> $file
        rm -f mem.txt
        echo -n "."
        echo -n 'Memory after '$job' (Heap [MB]) according to tcmalloc: ' >> $file
        PROFFILE=$what.$job.tcmprofile
        LD_PRELOAD="$TCM" HEAPPROFILE=$PROFFILE $what < $job.in >> /dev/null 2>&1
        echo -n "."
        PROFFILE1=`ls ${PROFFILE}.* | sort | tail -n 1`
        pprof --text `which $what` $PROFFILE1 | head -n1 | sed 's,^Total: ,,' >> $file
        rm ${PROFFILE}_*.heap
        echo '' >> $file
        echo -n ". "
    done
    echo ""
done

rm -f load.in run.in start.in 
cat cint.txt cling.txt
