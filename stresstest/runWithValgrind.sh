

if test "$1" = ""; then
    echo "usage: $0 application options"
    exit 0
fi

valgrind \
--suppressions=$ROOTSYS/etc/valgrind-root.supp --suppressions=$VALGRIND_SUPP_FILES/valgrind-suppression_ROOT_optional.supp \
--tool=memcheck --freelist-vol=500000000 --leak-check=yes --trace-children=yes --num-callers=12 -v --show-reachable=yes $* |& tee valgrind.log

#valgrind \--suppressions=$VALGRIND_SUPP_FILES/valgrind-root.supp --suppressions=$VALGRIND_SUPP_FILES/valgrind-suppression_ROOT_optional.supp \
#--tool=massif --trace-children=yes --num-callers=12 $* \
#>! /tmp/stelzer/valgrind_massif.log 2>&1"

