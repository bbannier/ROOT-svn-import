#!/usr/bin/env python

import os,re,sys

if __name__ == "__main__":

    p_tmva = re.compile('#include.+"TMVA/(.*).h')
    p_incl = re.compile('#include.+"(.*).h"')

    for f in [x for x in os.listdir("../src/") if (x[-4:]=='.cxx' and x!='TMVA_Dict.C')]:
        needsFixing = False
        fh = open("../src/%s"%f,"r")
        content = [l.rstrip('\n') for l in fh.readlines()]
        fh.close()
        for ln,l in enumerate(content):
            m = p_tmva.match(l)
            if m:
                preprop = "#ifndef.+ROOT_TMVA_%s" % m.group(1)
                if ln>0 and not re.match(preprop,content[ln-1]):
                    if content[ln-1].startswith('#ifndef'):
                        print "%s line %.5i:  wrong #ifndef statement" % (f,ln)
                        print "Is        :    '%s'" % content[ln-1]
                        print "Should be :    '#ifndef ROOT_TMVA_%s'" % m.group(1)
                        sys.exit(0)
