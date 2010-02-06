#!/usr/bin/env python

import os, sys, time
from optparse import OptionParser

def getOptions():
    parser = OptionParser(usage="usage: %prog options")
    parser.add_option( "-v", dest = "version", help = "new tag, e.g. 4.0.4, mandatory" )
    parser.add_option( "-d", dest = "dryrun", action = "store_true", default=False, help = "Just print, don't execute commands" )
    opts = parser.parse_args()[0]
    if opts.version==None or opts.version.count('.')!=2:
        parser.print_help()
        sys.exit(1)
    return opts

def updateVersionFile(new_release,dry):

    version = tuple([int(x) for x in opts.version.split('.')]);
    if dry:
        print "Updated the file 'inc/Version.h' to release", version
        return
    
    infile  = file("inc/Version.h","r")
    outfile = file("inc/Version.new.h","w")
    lines = infile.readlines()
    ltime = time.localtime()

    for line in lines:
        if line.find('TMVA_RELEASE ')!=-1:
            print>>outfile, '#define TMVA_RELEASE      "%i.%i.%i"' % version
        elif line.find('TMVA_RELEASE_DATE ')!=-1:
            print>>outfile, '#define TMVA_RELEASE_DATE "%s"' % time.strftime('%b %d, %Y', ltime)
        elif line.find('TMVA_RELEASE_TIME ')!=-1:
            print>>outfile, '#define TMVA_RELEASE_TIME "%s"' % time.strftime('%X', ltime)
        elif line.find('TMVA_VERSION_CODE ')!=-1:
            code  = version[0] << 16
            code += version[1] << 8
            code += version[2]
            print>>outfile, '#define TMVA_VERSION_CODE %i' % code
        else:
            print>>outfile, line,

    infile.close()
    outfile.close()
    os.system('mv inc/Version.new.h inc/Version.h')
    print "Updated the file 'inc/Version.h' to release", version


def update_dev_head(dry):
    print "Updating local version to head of development ..."
    cmdup = 'svn switch https://root.cern.ch/svn/root/branches/dev/tmva'
    if dry:
        print cmdup
    else:
        print getoutput(cmdup)

def commit_changes(new_release, dry):
    cmdci = 'svn ci -m "new release %s"' % new_release
    print "commiting ..."
    if dry:
        print cmdci
    else:
        print getoutput(cmdci)


if __name__ == "__main__":

    opts = getOptions()

    update_dev_head(opts.dryrun)
    
    updateVersionFile(opts.version, opts.dryrun)

    commit_changes(opts.version, opts.dryrun)
