#!/usr/bin/env python

import os,re,sys

if __name__ == "__main__":

    p_tmva = re.compile('#include.+"TMVA/(.*).h')
    p_incl = re.compile('#include.+"(.*).h"')

    for f in [x for x in os.listdir("../src/") if (x[-2:]=='.h' and x!='TMVA_Dict.h')]:
        needsFixing = False
        fh = open("../src/%s"%f,"r")
        content = [l.rstrip('\n') for l in fh.readlines()]
        fh.close()
        fh = open("../fixedheaders/%s"%f,"w")
        # first check if the #define matches the filename
        for ln,l in enumerate(content):
            if not l.startswith("#define "): continue
            if not re.match("^#ifndef ROOT_TMVA_%s$"%f[:-2], content[ln-1]):
                print "File %s, line %i: wrong ifndef statement '%s'" % (f,ln-1,l)
                sys.exit(0)
            if not re.match("^#define ROOT_TMVA_%s$"%f[:-2], l):
                print "File %s, line %i: wrong define statement '%s'" % (f,ln,l)
                sys.exit(0)
            break


        for ln,l in enumerate(content):
            m = p_tmva.match(l)
            if m:
                preprop = "#ifndef.+ROOT_TMVA_%s" % m.group(1)
                if ln>0 and not re.match(preprop,content[ln-1]):
                    if content[ln-1].startswith('#ifndef'):
                        print "Fix file %s line %.5i, it contains a wrong #ifndef statement" % (f,ln)
                        sys.exit(0)
                    print "%-40s %.5i: %s  =====> FIX" % (f,ln,l)
                    needsFixing = True
                    print >>fh, "#ifndef ROOT_TMVA_%s" % m.group(1)
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
