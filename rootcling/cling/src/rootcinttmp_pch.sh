#! /bin/bash

ret=0

if "$@"; then
    rootsys=$PWD
    shift 3 # rootcint -cint -f
    dict=`echo $1 | sed 's,.cxx,_dicthdr.h,'`
    pch=`echo lib/$(basename $1) | sed 's,.cxx,.pch,'`
    echo "" > $dict
    shift
    incl=""
    while test "x$1" != "x"; do
        case $1 in
            -I*) incl="$incl $1" ;;
            -*) ;;
            *) if test "x$incl" != "x"; then
                   echo "//" $incl > $dict
                   incl=""
               fi
               islinkdef=""
               if ! (echo $1 | grep -i linkdef > /dev/null 2>&1 ); then
                   # skip this specif file
                   if (echo $1 | grep VectorUtil_Cint.h > /dev/null 2>&1 ); then
                      shift
                      continue
                   fi
                   hdr=`echo $1 | sed 's,^'$rootsys',..,'`
                   hdr=`echo $hdr | sed 's,^include/,,'`
                   echo '#include "'$hdr'"' >> $dict
               fi
               ;;
        esac
        shift
    done

    CC1OPTS=`echo "" | clang -c -o /dev/null -v -x c++ - 2>&1 | \
             grep [-]cc1 | head -n1 | sed -e 's,-emit-obj ,,' \
                -e 's,-main-file-name - ,,' -e 's,-v ,,' \
                -e 's,-o /dev/null ,,' -e 's,c++ -,c++,' \
                -e 's,",,g'`

    echo "$CC1OPTS `grep '^// -I' $dict | sed 's,^//,,'` \
       -I. -Iinclude -emit-pch -relocatable-pch $dict -o $pch"
    $CC1OPTS `grep '^// -I' $dict | sed 's,^//,,'` \
       -I. -Iinclude -emit-pch -relocatable-pch $dict -o $pch
    ret=$?
    if [ $ret -ne 0 ]; then
       # force re-running of script in case of pch generation error
       rm -f $1
    else
       # by default created with 600
       chmod 644 $pch
    fi
fi

exit $ret
