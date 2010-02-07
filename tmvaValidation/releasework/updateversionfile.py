#!/usr/bin/env python

import os, sys, time, re
from optparse import OptionParser
from commands import getoutput as go

def getOptions():
    parser = OptionParser(usage="usage: %prog options")
    parser.add_option( "-v", dest = "version", help = "new tag, e.g. 4.1.4 (patches) or 4.2.0 (dev)" )
    parser.add_option( "-d", dest = "dryrun", action = "store_true", default=False, help = "print commands, don't execute them" )
    parser.add_option( "-l", action = "store_true", dest = "listtags", help = "list last 3 tags (does nothing else)" )
    opts = parser.parse_args()[0]
    if opts.listtags: return opts
    if opts.version != None and (opts.version.count('.')==2): return opts
    parser.print_help()
    sys.exit(1)

def update_version_file(new_release,dry):

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


def update(dry):
    print "Updating local version to head of development ..."
    cmdup = 'svn up'
    if dry:
        print cmdup
    else:
        print go(cmdup)

def commit_changes(new_release, dry):
    cmdci = 'svn ci -m "new release %s"' % new_release
    print "commiting ..."
    if dry:
        print cmdci
    else:
        print go(cmdci)

def get_base_path():
    url = go("svn info | grep '^URL'").splitlines()[0]
    p = re.compile("URL: (.*/branches/dev)(.*)/tmva")
    m=p.match(url)
    if not m:
        print "Must be in a tmva directory under branches/dev/..."
        sys.exit(1)
    pkgurl, flavor = m.groups()
    return pkgurl, flavor 

def list_tags(baseurl, lastn=3):
    cmdlt = "svn list %s/tmvatags" % baseurl
    tags = go(cmdlt).splitlines()
    for l in tags[-lastn:]:
        print l.rstrip('/')
    sys.exit(0)

def check_version(version,baseurl,flavor):
    v = tuple([int(x) for x in opts.version.split('.')]);
    cmdlt = "svn list %s/tmvatags" % baseurl
    tags = go(cmdlt).splitlines()
    tagtuples = [tuple(map(int, x.lstrip('V').rstrip('/').split('-'))) for x in tags]
    if v in tagtuples:
        print "Version %s already exists, can't use it again" % version
        sys.exit(1)
    if flavor=="" and v[2]!=0:
        print "Minor version (last digit) for development must be '0'"
        sys.exit(1)
    if flavor!="" and v[2]==0:
        print "Minor version (last digit) for bugfix release must not be '0'"
        sys.exit(1)

    
if __name__ == "__main__":

    opts = getOptions()

    baseurl, flavor = get_base_path()

    if opts.listtags:
        list_tags(baseurl)

    check_version(opts.version,baseurl, flavor)

    update(opts.dryrun)
    
    update_version_file(opts.version, opts.dryrun)

    commit_changes(opts.version, opts.dryrun)
