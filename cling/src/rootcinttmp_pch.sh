#! /bin/bash
if "$@"; then
    shift 3 # rootcint -cint -f
    dict=cint/cling/src/`basename $1 | sed 's,.cxx,_dicthdr.h,'`
    echo "" > $dict
    shift
    incl=""
    while test "x$1" != "x"; do
        case $1 in
            -I*) incl="$incl $1" ;;
            -*) ;;
            *) if test "x$incl" != "x"; then echo "//" $incl > $dict;  incl=""; fi
               islinkdef=""
               if ! (echo $1 | grep -i linkdef > /dev/null 2>&1 ); then
                   echo '#include "'$1'"' >> $dict
               fi
               ;;
        esac
        shift
    done
fi
