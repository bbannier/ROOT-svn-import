#!/usr/bin/env python
# @(#)root/tmva $Id$
# ------------------------------------------------------------------------------ #
# Project      : TMVA - a Root-integrated toolkit for multivariate data analysis #
# Package      : TMVA                                                            #
# Python script: CreateRootVersionRef.py                                         #
#                                                                                #
# Creation of reference html pages for correspondance between ROOT and TMVA      #
# versions                                                                       #
#                                                                                #
# For help type "python CreateRootVersionRef.py --help"                          #
# ------------------------------------------------------------------------------ #

# --------------------------------------------
# Standard python import
import sys,time,urllib
#

# default settings
ROOT_SNV_RELDIR  = "http://root.cern.ch/svn/root/tags/"
DEFAULT_OUTFNAME = "versionRef.html"

# color definitions
columncols = [ 'lavender', 'lightsteelblue' ]

def readRootVersions():
    # 
    # first read in all ROOT releases
    rtagfile = urllib.urlopen( ROOT_SNV_RELDIR )
    roottaglist = []
    taglist     = [[]]
    while 1:
        line = rtagfile.readline()
        if not line: break    

        # example line: <li><a href="v5-15-08-caf/">v5-15-08-caf/</a></li>
        if line.find('href="v') > 0:
            roottaglist.append( line[line.find('/">')+3:line.find('</a>')] )

    # now search tmva line
    for tag in roottaglist:
        rmodfile = urllib.urlopen( ROOT_SNV_RELDIR + "/" + tag)
        
        tmva_version = ''

        while 1:
            line = rmodfile.readline()
            if not line: break    
            if line.find('tmva') > 0:
                
                rtmvafile = urllib.urlopen( ROOT_SNV_RELDIR + "/" + tag + '/' + 'tmva/inc/Version.h' )

                while 1:
                    l = rtmvafile.readline()
                    if not l: break    

                    if l.find(' TMVA_RELEASE ') > 0:
                        tmva_version = l.replace('TMVA_RELEASE','').replace('#define','').replace('"','').strip()

                
        if tmva_version != '':
            taglist.append( [tag.replace('/',''), tmva_version] )

    return taglist

# ----------------------------------------------------------
# Main routine
def main():
    # read version list first
    print 'Scanning svn web directories...'
    taglist = readRootVersions()
    print 'done !'

    outfname   = DEFAULT_OUTFNAME
    oldstdout  = sys.stdout
    sys.stdout = open( outfname, 'w' )

    print '<!DOCTYPE html PUBLIC "-//W3C//DTD HTML 4.01 Transitional//EN"><html xmlns:my><head>'
    print '<title>' + 'Classifier Info' + '</title>'
    print '<LINK href="CreateOptionRef.css" rel="stylesheet" type="text/css">'
    print '<body style="background-color: #ffffff;">'
    print '<table class=mytable width=450><tr>'
    print '<th class=alltitle style="font-family: sans-serif; font-size: 90%;"> TMVA Versions in ROOT Releases: </th>'
    print '</tr><tr><td></td></tr><tr><td>'            
    print '</tr><tr><td></td></tr><tr><td>'            
    print '<ul>'
    print '<table class="mytable" width=90% style="padding: 0.4em;border: 3px lightgray solid;""><tr><th>ROOT release</th><th>Contains TMVA version</th></tr>' 
    #    print '<table class="mytable" width=90% style="padding: 0.4em;border: 3px lightgray solid;""><tr><th>ROOT release</th><th>Contains TMVA version</th><th rowspan=%i><img border=0 src="./images/arrow.png" height="18px" width="18px" alt="Information on classifier tuning"/></th></tr>' % len(taglist)+1
    tagref = ''
    ic = 1
    for tag in reversed(taglist):
        if len(tag) > 0:
            if tag[1] != tagref:
                ic += 1
            bgcol = columncols[ic%2]
            tagref = tag[1]
            troot = '&nbsp;&nbsp;<a href="' + ROOT_SNV_RELDIR + tag[0] + '">' + tag[0].replace('v','') + '</a>'
            ttmva = '&nbsp;&nbsp;<a href="' + ROOT_SNV_RELDIR + tag[0] + '/tmva">' + tag[1] + '</a>'
            print '<tr><td bgcolor=' + bgcol + '>' + troot + '</td><td bgcolor=' + bgcol + '>' + ttmva + '</td></tr>'            

    print '</table>'
    print '</ul>'
    print '</tr><tr><td></td></tr><tr><td>'            
    print '<p></p>'
    print '</ul><hr><font color="#555555">Created on %s (TMVA, 2005 - 2009)</font>' % time.ctime()
    print '</td></tr></table></body></html>'

    sys.stdout = oldstdout
    print 'Created output file: %s' % outfname

if __name__ == "__main__":
    main()

