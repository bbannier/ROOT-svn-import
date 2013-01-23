#!/usr/bin/env python
# @(#)root/tmva $Id$
# ------------------------------------------------------------------------------ #
# Project      : TMVA - a Root-integrated toolkit for multivariate data analysis #
# Package      : TMVA                                                            #
# Python script: CreatePythonRef.py                                              #
#                                                                                #
# Creation of reference html pages for configuration options and                 #
# MVA method tuning and the .tex input files for the UsersGuide                  #
#                                                                                #
# It requires prior creation of the reference text files via running:            #
#                                                                                #
#     root -l DumpOptionsReference.C                                             #
#                                                                                #
# For help type "python CreatePythonRef.py --help"                               #
# ------------------------------------------------------------------------------ #

# --------------------------------------------
# Standard python import
import sys, os, time, urllib
import getopt # command line parser
import glob   # read files from directory

# --------------------------------------------

# global settings 

# colors: [light, dark]
classifierSharedCol   = [ '#D9FAC9', '#B0FA91', '#566D7E' ]
classifierSpecificCol = [ 'lavender', 'lightsteelblue', '#566D7E' ]
fitterCol             = [ '#FFF8C6', '#FFF39D', '#827B60' ]
otherCol              = [ '#FFDFDA', '#EEC1C0', '#7F462C' ]

# class for configurable object
class Configurable:
    "Container for configurable object"
    def __init__( self, name, descr ):
        self.optionlist   = []
        self.name         = name.replace('Method','MVA::').replace('FDA_','')
        if self.name == 'DataSet':
            self.name = self.name.replace('DataSet','Factory::PrepareForTrainingAndTesting' )
        self.description  = descr
        self.helptext     = ''
        self.type         = 'other'
        if self.name.startswith( 'MVA' ):
            self.type     = 'MVA'
        elif self.name.find( 'Fitter_' ) != -1:
            self.name     = 'Fitter_' + self.name.partition('Fitter_')[2]
            self.type     = 'fitter'
        else:
            # for PDF, remove 'likelihood' in name
            if 'PDF' in self.name: self.name = 'PDF'
        self.isChecked    = False

    def getName( self ):
        return self.name;

    def getOptions( self ):
        return self.optionlist

    def checkForSharedOptions( self, configurables ):
        for option in self.optionlist:
            found = True
            for c in configurables:
                if c.type == 'MVA':
                    f = False
                    if self.name != c.getName:
                        for otheroption in c.getOptions():                        
                            if option == otheroption:
                                f = True
                    found &= f
            if found:
                option.setShared( True )
            else:
                option.setShared( False )
        self.isChecked = True

    def setHelpText( self, t ):
        self.helptext = t

    def appendOption( self, option ):
        self.optionlist.append( option )    

    def printOptions( self, type ):
        # determine color of table rows for this class type
        color = otherCol
        if self.type == 'MVA':
            color = classifierSpecificCol
        elif self.type == 'fitter':
            color = fitterCol

        text = 'none'
        if type.lower() == 'html': 
            print '<a name="%s"></a><table class="mytable">' % self.name
            print '<tr>' 
            n = self.name
            text = 'Configuration options reference for class: <i>' + n + '</i>'
            if self.type == 'MVA':
                text = 'Configuration options reference for MVA method: <i>' + n.replace('MVA::','') + '</i>'
            elif self.type == 'fitter':
                fitmethod = 'Unknown'
                if n.find('_GA') > 0:
                    fitmethod = 'Genetic Algorithm (GA)'
                elif n.find('_SA') > 0:
                    fitmethod = 'Simulated Annealing (SA)'
                elif n.find('_MT') > 0 or n.find('_Minuit') > 0 :
                    fitmethod = 'TMinuit (MT)'
                elif n.find('_MC') > 0:
                    fitmethod = 'Monte Carlo sampling (MC)'

                text = 'Configuration options reference for fitting method: <i>' + fitmethod + '</i>'

            if self.type == 'MVA':
                print '<th colspan=4 class=alldescr>' + self.description + ' :</th>' 
                print '<td align="right"><a STYLE="text-decoration: none" href="javascript:openWindow(\'%s\',\'%s\')" title="Tips how to tune the %s method"><img border=0 src="./images/i_icon.png" height="18px" width="18px" alt="Information on method tuning"/></a></td>' % (self.name, self.helptext, self.name.replace('MVA::',''))
            else:
                print '<th colspan=5 class=alldescr>' + self.description + ' :</th>' 
            print '</tr><tr>' 
            print '<th colspan=5 class=alltitle>' + text + '</th>'
            print '</tr><tr>' 
            columns = ['Option', 'Array', 'Default value', 'Predefined values', 'Description' ]
            tstr = ''
            for c in columns:
                tstr += '<th class=coltitle>' + c + '</th> '
            print tstr
            print '</tr>'
        elif type.lower() == 'text':
            print 'Options of configurable "%s":' % self.name
            print '----------------------------------------------'
            
        # loop over options        
        for i, option in enumerate(self.optionlist):
            if type.lower() == 'text': 
                option.printOptionText()
            elif type.lower() == 'html': 
                print '<tr>'
                option.printOptionHtml( i, color[0], color[1] )
                print '</tr>'
            else:
                print >> sys.stderr, 'ERROR: unknown print format "%s"' % type
                abort()

        if type.lower() == 'html': 
            print '<tr><td colspan=5><p></p></td></tr>'
            print '</table>'

        # == l a t e x : begin  ================================================================================

        # create in any case also latex output 
        # print to file
        dname = DEFAULT_OUTTEXDIR
        if not os.path.exists( dname ): os.makedirs( dname )

        oldstdout   = sys.stdout        
        latexfname  = dname + '/' + self.name.replace('::','__') + '.tex'
        sys.stdout  = open( latexfname, 'w' )

        print '\\begin{optiontableAuto}'

        # begin of actual table
        notYet    = True
        wasShared = False
        for i, option in enumerate(self.optionlist):
            #if option.isShared: wasShared = True
            #if not option.isShared and notYet and wasShared:
            #    print '&&&& \\\\[-0.45cm] \\hline'
            #    notYet = False
                
            if not option.isShared: option.printOptionLatex( option == self.optionlist[-1] )        
        # end of table

        print '\\end{optiontableAuto}'
        # don't create label
        # print '\\label{opt:%s}' % self.name.lower().replace('MVA::','')
        # print '\\caption[.]{\optionCaptionSize '
        # print '     ' + text.replace('<i>','{\em ').replace('</i>','}') + '.'
        # if self.type == 'MVA':
        #     print '     All options above the horizontal line are shared among all methods (through common base class).'
        #     print '     The options below the horizontal line are specific.'
        
        sys.stdout = oldstdout

        # for one method create also Base class table (choose method: Fisher)
        if "Fisher" in self.name:
            oldstdout   = sys.stdout        
            latexfname  = dname + '/' + (self.name.replace('::','__') + '.tex').replace('Fisher','MethodBase')
            sys.stdout  = open( latexfname, 'w' )
            
            print '\\begin{optiontableAuto}'

            # begin of actual table
            lastShared = ""
            for i, option in enumerate(self.optionlist):
                if option.isShared: lastShared = option
            for i, option in enumerate(self.optionlist):
                if option.isShared: option.printOptionLatex( option == lastShared )        
              
            print '\\end{optiontableAuto}'
            # don't create label here
            # print '\\label{opt:%s}' % self.name.lower().replace('MVA::','').replace('fisher','methodbase')

            sys.stdout = oldstdout

        # == l a t e x : end  ==================================================================================
        
    def createHelpPages():
        print '<SCRIPT LANGUAGE="JavaScript">'
        print '<!--//hide script from old browsers'
        print 'window.open("newpage.htm", "new_win", "resizable=yes, scrollbars=no, toolbar=no, location=no, directories=no, status=yes, menubar=no, width=520, height=200, top=5, left=5")'
        print '//end hiding contents -->'
        print '</SCRIPT>'

# class holds option
class Option:
    "Container for single option"
    def __init__( self, inputLines ):
        # analyse lines
        # format: first line is option
        #         if first line contains array [0] -> second++ line has additional elements
        #         second line can also indicate whether predefined values exist        
        #         third++ lines give predefined values
        # -------
        # interpret first line
        line = inputLines[0]
        l2 = line.split('"')
        if len(l2) < 3:
            print >> sys.stderr, 'ERROR: too small number of "-element splits'
            print l2
            abort()
        elif len(l2) > 3:
            # concatenate everything after the third split element in case 
            # the info containted a "'"'
            for l in l2[3:len(l2)]:
                l2[2] += l

        option = l2[0].strip().replace(':','')

        # check for presence of array
        nextLineIndex = 1
        self.isArray = False
        if option.find('[0]') != -1:
            option = option.rsplit('[0]',1)[0]
            self.isArray = True
            # remove following (trivial) lines
            for k, line in enumerate(inputLines[nextLineIndex:len(inputLines)]):                
                if line.find( '[' + str(k+1) + ']' ) != -1:
                    nextLineIndex += 1

        # define member variables
        self.opt      = option        
        self.isShared = False
        self.val      = l2[1].strip()
        # replace by non-scientific style if number
        try:
            v = float(self.val)
            self.val = "%g" % v
        except:
            pass
        self.info     = l2[2].strip().replace('[','').replace(']','')
        self.predefs  = []

        # interpret second line
        if len(inputLines) > nextLineIndex:
            line = inputLines[nextLineIndex]
            if not line.startswith( 'PreDefined' ):
                print >> sys.stderr, 'ERROR: next line in inputLines "%s" does not begin with "PreDefined"' % line
                abort()
            for line in inputLines[nextLineIndex+1:len(inputLines)]:
                st = line.replace('-','').strip()
                if len(st) > 0:
                    self.predefs.append( st )

    def __eq__( self, other ):
        if self.opt == other.opt and self.info == other.info:
            return True
        else:
            return False

    def setShared( self, shared ):
        self.isShared = shared

    def printOptionText( self ):
        array = 'No '
        if self.isArray: array = 'Yes'
        print 'Option: %25s  |  Value: %15s  |  Array: %s  |  Info: %s' % (self.opt, self.val, array, self.info)
        if len(self.predefs) > 0:
            print '           Predefined values  : ', self.predefs

    def printOptionLatex( self, lastElement ):
        array = '\mc{1}{c}{--}'
        if self.isArray: array = 'Yes'
        pstr = '\mc{1}{l}{--}'
        if len(self.predefs) > 0:
            pstr = ''
            for p in self.predefs:
                pstr += p + ', '
            pstr = pstr.rsplit(',',1)[0]

        lbreak = ''
        if not lastElement: lbreak = '\\\\'
        s = '%25s  &  %s  &  %15s  &  %s  &  %s %s' % (self.opt, array, self.val, pstr, self.info, lbreak)
        print s.replace('_','\_').replace('#','\#').replace('^','**').replace('>','$>$').replace('<','$<$').replace('%','\%')


    def printOptionHtml( self, num, lightCol, darkCol ):
        array = 'No '
        if self.isArray: array = 'Yes'
        pstr = '&minus;'
        if len(self.predefs) > 0:
            pstr = ''
            for p in self.predefs:
                pstr += p + ', '
            pstr = pstr.rsplit(',',1)[0]

        if num%2 == 0:
            b = 'bgcolor=' + darkCol
            if self.isShared:
                b = 'bgcolor=' + classifierSharedCol[1]
        else:
            b = 'bgcolor=' + lightCol
            if self.isShared:
                b = 'bgcolor=' + classifierSharedCol[0]
        print '         <td %s class=option>%25s</td> <td %s class=array>%s</td> <td %s class=val>%20s</td> <td %s class=predef>%s</td> <td %s class=info>%s</td>' % (b, self.opt, b, array, b, self.val, b, pstr, b, self.info)


# Default settings for command line arguments
DEFAULT_OUTFNAME   = 'optionRef.html'
DEFAULT_OUTTEXDIR  = 'optiontables'
DEFAULT_INDIR      = 'optionInfo'
DEFAULT_VERSIONREF = 'versionRef.html'

# Print usage help
def usage():
    print ' '
    print 'Usage: python %s [options]' % sys.argv[0]
    print '  -i | --inputdir    : input directory of option reference files (default: "%s")' % DEFAULT_INDIR
    print '  -o | --outputfile  : name output file; file extension defines format: "txt" or "html" (default: "%s")' % DEFAULT_OUTFNAME
    print '  -v | --verbose'
    print '  -? | --usage      : print this help message'
    print '  -h | --help       : print this help message'
    print ' '

def abort():
    print 'Abort execution.'
    sys.exit(1)

def getTMVAVersion():
#    reldir = 'tmva'
    reldir = os.environ['TMVASYS']
    for i in range(4):
        if os.path.isdir( reldir ):
            v = reldir + '/inc/TMVA/Version.h'
            if not os.path.isfile( v ):
                return '-<i><font color="red">unknown</font></i>'
            file = open( v, 'r' )
            for line in file:                
                if line.find(' TMVA_RELEASE ') > 0:
                    return line.replace('TMVA_RELEASE','').replace('#define','').replace('"','').strip()
        else:
            reldir = '../' + reldir

    # no version found
    return '-<i><font color="red">unknown</font></i>'

# Main routine
def main():

    try:
        # retrive command line options
        shortopts  = "i:o:vh?"
        longopts   = ["inputdir=", "outputfile=", "verbose", "help", "usage"]
        opts, args = getopt.getopt( sys.argv[1:], shortopts, longopts )

    except getopt.GetoptError:
        # print help information and exit:
        print >> sys.stderr, 'ERROR: unknown options in argument %s' % sys.argv[1:]
        usage()
        sys.exit(1)

    indirname   = DEFAULT_INDIR
    outfname    = DEFAULT_OUTFNAME
    verbose     = False
    for o, a in opts:
        if o in ("-?", "-h", "--help", "--usage"):
            usage()
            sys.exit(0)
        elif o in ("-i", "--inputdir"):
            indirname = a
        elif o in ("-o", "--outputfile"):
            outfname = a
        elif o in ("-v", "--verbose"):
            verbose = True

    # sanity check
    outtype = ''
    if outfname.rfind('.txt') > 0:
        outtype = 'txt'
    elif outfname.rfind('.html') > 0:
        outtype = 'html'
    else:
        print >> sys.stderr, 'ERROR: unknown format (file extension) in output file "%s". Allowed are the extensions: "txt", "html"' % outfname
        abort()

    # create list of configurables
    configurables = []

    # read files
    filelist = glob.glob( indirname + '/*.txt' )
    if filelist == []: 
        print 'Warning: no input "*.txt" files in directory "%s/": check directory name and path' % indirname
        abort()

    # veto following files
    vetolist = [ 'Configurable_', 'Likelihood PDF Bkg', 'Likelihood PDF Sig' ]

    # iterate through files, read options and create configurables
    print 'Looping through reference files:'
    for fname in filelist:
        # check if file start with "Configurable" --> ignore
        cont = False
        for v in vetolist: 
            if fname.partition('/')[2].startswith(v): cont = True
        if cont: continue

        print '...opening %s' % fname
        file = open( fname, 'r' )

        # check sanity of file
        patterns = [ '# List of options:', '# Configurable:', '# Description:' ]
        names    = []
        for p in patterns:
            line = file.readline()
            if not line.startswith( p ):
                print >> sys.stderr, 'ERROR: bad file format, first line "%s" should start with "%s"' % (line, p)
                abort()
            names.append( line.replace(p,'').strip() )

        configurable = Configurable( names[1], names[2] )

        # read lines
        lines = []
        for line in file:
            # check if in 'help' section
            if line.find( 'Help for MVA method' ) > 0:
                break;
            elif not line.startswith( '# -----------------------------' ):
                lines.append( line )
            else:
                configurable.appendOption( Option( lines ) )
                del lines[:]

        # interpret help section
        helptext = '<!DOCTYPE html PUBLIC "-//W3C//DTD HTML 4.01 Transitional//EN"><html xmlns:my><head>'
        helptext += '<title>' + 'MVA method Info' + '</title>'
        helptext += '<LINK href="CreateOptionRef.css" rel="stylesheet" type="text/css">'
        helptext += '<body style="background-color: #ffffff;">'
        helptext += '<table class="mytable"><tr>'
        helptext += '<th class="coltitle" style="font-family: sans-serif; font-size: 90%;">Tuning information for MVA method: ' + configurable.getName().replace('MVA::','') + '</th>'
        helptext += '</tr><tr><td></td></tr><tr><td>'
        first    = True
        filterout = [ 'Short description:', 'Performance optimisation:', 'Performance tuning via configuration options:',
                      'Use of booking options:']
        for line in file:
            if line.find('# End of Message___') > 0:
                break

            c = line[line.find(':')+1:len(line)].strip()
            # look out for end signal
            if c == '' and first:
                continue
            elif c == '':
                c = '<br><p></p>'
            else:
                first = False
                for filter in filterout:
                    if c.find(filter) > 0:
                        c = '</ul><b>' + filter + '</b> <ul>'
                        first = True
            helptext += c + ' '

        helptext += '</ul><hr><font color="#555555">Created on %s (TMVA, 2006&minus;2011)</font>' % time.ctime()
        helptext += '</td></tr></table></body></html>'
        configurable.setHelpText( helptext.replace('"','&quot;' ) )

        configurables.append( configurable )

    # first thing, sort configurable list
    # first MVA methods, then fitters, the other
    ctmp = []
    for c in configurables:                
        if c.type == 'MVA':
            ctmp.append( c )
    for c in configurables:                
        if c.type == 'fitter':
            ctmp.append( c )
    for c in configurables:                
        if c.type == 'other':
            ctmp.append( c )
    del configurables[:]    
    configurables = ctmp
        
    # find shared options
    for c in configurables:                
        if c.type == 'MVA':
            c.checkForSharedOptions( configurables )

    # print to file
    oldstdout = sys.stdout
    sys.stdout = open( outfname, 'w' )
    if outtype == 'html':
        print '<!DOCTYPE html PUBLIC "-//W3C//DTD HTML 4.01 Transitional//EN">'
        print '<html xmlns:my>'
        print '<head>'
        print '<title>TMVA Options Reference</title>'
        print '<meta http-equiv="content-type"'
        print '  content="text/html; charset=ISO-8859-1">'
        print '<meta name="TMVA" content="TMVA">'
        print ' '
        print '<LINK href="CreateOptionRef.css" rel="stylesheet" type="text/css">'
        print ' ' 
        print '<!-- include js functions -->'
        print '<script src="ow.js"></script>'
        print ' '
        print '<!-- javascript to open pop-up window -->'
        print '<script language="JavaScript">'
        print 'function openWindow(name,text) { '
        print 'win = window.open("", name, "width=620,height=510,scrollbars=yes");'
        print 'win.document.writeln( text ); }'
        print 'function openPageWindow(link,name) { '
        print 'win = window.open(link, name, "width=620,height=510,scrollbars=yes"); }'
        print '</script>'
        print ' '
        print '<!-- favicon -->'
        print '<link rel="shortcut icon" href="http://hoecker.web.cern.ch/hoecker/htdocs/favicon.ico" />'
        print '<link rel="icon" href="http://hoecker.web.cern.ch/hoecker/htdocs/favicon.ico" />'
        print '</head>'
        print '<!-- FFA521 -->'    
        print ' '
        print '<body style="background-color: #ffffff;">'
        print '<center>'
        print '<table border="0"  cellpadding="2" cellspacing="0" width="800"  bgcolor="#ffffff">'
        print '<tr><td bgcolor=gainsboro> <table width=100%><tr><td><font size=+1><b>TMVA Configuration Options Reference</b>'
        vtmva = getTMVAVersion()
        print '</font></td><td align="right"><a style="text-decoration:none" href="http://tmva.sf.net" title="Download this version of TMVA"><font color="red">Reference version: TMVA-v' + vtmva + '</font></a>'
        print '</td>'
        print '<td align="right"><a STYLE="text-decoration: none" href="javascript:openPageWindow(\'%s\',\'%s\')" title="Reference for TMVA versions in ROOT releases">TMVA-version @ ROOT</a></td>' % (DEFAULT_VERSIONREF,'ROOT vs TMVA Version Reference')
        print '</tr></table></td></tr>'
        print '<tr><td><br>'
        print 'Reference for configuration options defined in the option string of each MVA method booking, and for the definition of data sets used for training and testing (Factory).<br>'
        print '<p></p>'

        print 'Table fields:'
        print '<p></p>'
        print '<table><tr>'
        print '<td width=10></td><td width=130><b>Option:</b></td>'
        print '<td>The option identifier in the option string (given, e.g., in \"factory->BookMethod(...)\" call).</td>'
        print '</tr><tr>'
        print '<td width=10></td><td><b>Array:</b></td>'
        print '<td>Can the option be set individually for each input variable via the "[i]" tag, where "i" is the ith variable?</td>'
        print '</tr><tr>'
        print '<td width=10></td><td><b>Default value:</b></td>'
        print '<td>Value used if option is not explicitly set in the configuration option string.</td>'
        print '</tr><tr>'
        print '<td width=10></td><td><b>Predefined values:</b></td>'
        print '<td>Options can be categories of predefined values among which the user must choose.</td>'
        print '</tr><tr>'
        print '<td width=10></td><td><b>Description:</b></td>'
        print '<td>Info about the option.</td>'
        print '</tr></table>'
        print '<br>Colour codes:<br>'
        print '<p></p>'
        print '<table class="mytable"><tr>'
        print '<td width=10></td><td bgcolor=' + classifierSharedCol[0] + '><b>Greenish rows:</b></td>'
        print '<td bgcolor=' + classifierSharedCol[0] + '>Options shared by all MVA methods (through common base class).</td>'
        print '</tr><tr>'
        print '<td width=10></td><td width=130 bgcolor=' + classifierSpecificCol[0] + '><b>Bluish rows:</b></td>'
        print '<td bgcolor=' + classifierSpecificCol[0] + '>Specific MVA options.</td>'
        print '</tr><tr>'
        print '<td width=10></td><td bgcolor=' + fitterCol[0] + '><b>Yellowish rows:</b></td>'
        print '<td bgcolor=' + fitterCol[0] + '>Configuration options for minimiser (fitter) classes.</td>'
        print '</tr><tr>'
        print '<td width=10></td><td bgcolor=' + otherCol[0] + '><b>Redish rows:</b></td>'
        print '<td bgcolor=' + otherCol[0] + '>Options for other configurable classes.</td>'
        print '</tr></table>'
        print '<tr><td>'
        print '<br>Available MVA methods (1st row), minimisation tools (2nd row), and other configurables (3rd row):<br>'
        print '<p></p>'
        print '<table class="mytable">'
        print '<tr><td width=10 ></td>'
        print '<td bgcolor=gainsboro>'
        nstr = ''
        for c in configurables:
            if c.type == 'MVA':
                nstr += '<a href="#' + c.getName() + '">[' + c.getName() + ']</a> '
        print nstr
        print '</td></tr>'        
        print '<tr><td width=10 ></td>'
        print '<td bgcolor=gainsboro>'
        nstr = ''
        for c in configurables:
            if c.type == 'fitter':
                nstr += '<a href="#' + c.getName() + '">[' + c.getName() + ']</a> '
        print nstr
        print '</td></tr>'        
        print '<tr><td width=10 ></td>'
        print '<td bgcolor=gainsboro>'
        nstr = ''
        for c in configurables:
            if c.type == 'other':
                nstr += '<a href="#' + c.getName() + '">[' + c.getName() + ']</a> '
        print nstr
        print '</td></tr>'        
        print '</table>'
        print '<br>'
        print '<tr><td> '
        for c in configurables:
            c.printOptions( outtype )
        print '</td></tr>'
        print '<tr><td>'
        print '<hr>'
        print '</td></tr>'
        print '<tr><td><table border=0 cellpadding=0 cellspacing=0 width=100%><tr><td>'
        print '<font color="#555555">Page created on %s (&copy; TMVA, 2006&minus;2011)</font></td>' % time.ctime()
        print '<td align="right"><a href="http://tmva.sf.net"><img border=0 vspace=0  width="80" src="images/tmva_logo.gif"></a>'
        print '</td></tr></table>'
        print '</td></tr>'
        print '</table>'
        print '</center>'
        print '</body>'
        print '</html>'

    elif outtype == 'txt':
        for c in configurables:
            c.printOptions( outtype )

    sys.stdout = oldstdout
    print 'Created output file: %s' % outfname
    print 'LaTex output should have been writting to optiontables/'
# ----------------------------------------------------------

if __name__ == "__main__":
    main()
