#!/bin/bash


igprof -d -pp -z -o igprof.pp.gz $PWD/$@ >& igtest.pp.log 
igprof -d -mp -z -o igprof.mp.gz $PWD/$@ >& igtest.mp.log
igprof-analyse --sqlite -d -v -g igprof.pp.gz | sqlite3 "$1_prf.sql3"
igprof-analyse --sqlite -d -v -g igprof.mp.gz | sqlite3 "$1_mem.sql3"

rm -f igprof.pp.gz
rm -f igprof.mp.gz


