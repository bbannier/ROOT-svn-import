echo "Setting up valgrind ..."
source /afs/cern.ch/sw/lcg/external/valgrind/3.2.3/slc4_amd64_gcc34/_SPI/start.sh

echo "valgrind --suppressions=valgrind-root.supp --suppressions=valgrind-suppression_ROOT_optional.supp --tool=memcheck --leak-check=yes --trace-children=yes --num-callers=12 --show-reachable=yes stressTMVA >! /tmp/stelzer/valgrind.log 2>&1"
echo "valgrind --suppressions=valgrind-root.supp --suppressions=valgrind-suppression_ROOT_optional.supp --tool=massif --trace-children=yes --num-callers=12 stressTMVA >! /tmp/stelzer/valgrind_massif.log 2>&1"
