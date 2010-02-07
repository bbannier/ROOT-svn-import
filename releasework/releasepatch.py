#!/usr/bin/env python

import os, sys, time, re
from optparse import OptionParser
from commands import getoutput as go

def getOptions():
    parser = OptionParser(usage="usage: %prog options")
    parser.add_option( "-p", dest = "patchrel", help = "patch branch, e.g. 5.26" )
    parser.add_option( "-d", dest = "dryrun", action = "store_true", default=False, help = "print commands, don't execute them" )
    parser.add_option( "-m", dest = "logmessage", help = "log message. If not supplied, take the last svn log" )

    opts = parser.parse_args()[0]
    if opts.patchrel != None and (opts.patchrel.count('.')==1): return opts
    parser.print_help()
    sys.exit(1)


def get_patch_paths(patchversion):
    pv = tuple(map(int,patchversion.split('.')))
    pa_tmva = "https://root.cern.ch/svn/root/branches/dev/tmvapatches/v%i-%i-00-patches/tmva" % pv
    pa_root = "https://root.cern.ch/svn/root/branches/v%i-%i-00-patches/tmva" % pv
    return pa_tmva, pa_root 


def has_changes(fr, to):
    #cmd = "svn diff -x -w %s %s" % (fr,to)
    #print go(cmd)
    #print "\nIn Summary"
    cmd = "svn diff --summarize %s %s" % (fr,to)
    output = go(cmd)
    if len(output)==0:
        print "No changes between\n  %s\nand\n  %s" % (fr,to)
        return False
    print output
    return True


def apply_changes(pa_root, pa_tmva, logmessage, dryrun):

    tmpdir = "/tmp/%s/tmvapatch" % os.environ["USER"]
    print "Temporary work dir:",tmpdir
    go("rm -rf %s" % tmpdir)
    os.mkdir(tmpdir)
    os.chdir(tmpdir)
    cmdco = "svn -q co %s" % pa_root
    print cmdco
    if not dryrun:
        print go(cmdco)
        os.chdir("tmva")

    cmdmg = "svn merge %s %s" % (pa_root, pa_tmva)
    print cmdmg
    if not dryrun:
        print go(cmdmg)
    
    cmdci = 'svn ci -m "%s"' % logmessage
    print cmdci
    if not dryrun:
        print go(cmdci)


def get_last_logentry(pa_tmva):
    cmd = "svn log -l 1 %s" % pa_tmva
    output = go(cmd).splitlines()
    rec = False
    for x in output:
        if rec: return x
        if len(x)==0 and not rec: rec=True
    print "No log found"
    sys.exit(0)


if __name__ == "__main__":

    opts = getOptions()

    pa_tmva, pa_root = get_patch_paths(opts.patchrel)

    if not has_changes(pa_root, pa_tmva):
        sys.exit(0)

    if opts.logmessage:
        logmessage = opts.logmessage
    else:
        logmessage = get_last_logentry(pa_tmva)

    apply_changes(pa_root, pa_tmva, logmessage, opts.dryrun)
