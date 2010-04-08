#!/bin/bash

let i=0
rm load.in
rm run.in
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

for what in $runwhat; do
    echo Running $what...
    file=${what}.txt
    echo '===' $what '===' > $file

    for job in start load load run; do
        echo '' >> $file
        echo $job ' time:' >> $file
        time ($what < $job.in > /dev/null 2>&1 ) 2>> $file
        echo 'Memory after '$job' (RSS,VSZ [kB]) according to ps:' >> $file
        cat mem.txt >> $file
        echo 'Memory after '$job' (Heap [MB]) according to massif:' >> $file
        valgrind --tool=massif --massif-out-file=${what}.${job}.massif  $what < $job.in >> /dev/null 2>&1
        ms_print ${what}.${job}.massif| grep -E '^[[:digit:].]+\^'|cut -d^ -f1 >> $file
        echo '' >> $file
    done
done
