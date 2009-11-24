#!/usr/bin/env python

import os,re

if __name__ == "__main__":

    p_tmva = re.compile('#include.+"TMVA/')
    p_incl = re.compile('#include.+"(.*).h"')

    for f in [x for x in os.listdir("../src/") if (x[-2:]=='.h' and x!='TMVA_Dict.h')]:
        needsFixing = False
        fh = open("../src/%s"%f,"r")
        content = [l.rstrip('\n') for l in fh.readlines()]
        fh.close()
        fh = open("../fixedheaders/%s"%f,"w")
        for ln,l in enumerate(content):
            m = p_incl.match(l)
            if m and not p_tmva.match(l):
                preprop = "#ifndef.+ROOT_%s" % m.group(1)
                if ln>0 and not re.match(preprop,content[ln-1]):
                    print "%-40s %.5i: %s  =====> FIX" % (f,ln,l)
                    needsFixing = True
                    print >>fh, "#ifndef ROOT_%s" % m.group(1)
                    print >>fh, l
                    print >>fh, "#endif"
                else:
                    print >>fh, l
                    pass
            else:
                print >>fh, l
                pass
        fh.close()
        if not needsFixing:
            os.remove("../fixedheaders/%s"%f)
