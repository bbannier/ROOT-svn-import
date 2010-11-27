
# This file contains helper functions for 'scan.py' for parsing an
# option string, which uses intervals [min,max,step] and lists
# {entry1,entry2,entry3}.  See 'scan.py' or README for more details.
#
# Note that the modules 'numpy' and 're' are needed to use this
# module.


import re, numpy


def concat(arg, blank=" "):
    """
    This function returns all possible concatenations of all elements
    of the list 'arg'.
    
    Parameters:
    - arg   - List of lists of strings to concat
    - blank - separator string between concated objects
    
    Returns:
    - List of list of all possible concatenations of the argument
      lists

    Example:

    a = ['1', '2']
    b = ['x', 'y', 'z']
    print concat([a,b]," ")
    
    Output: [['1 x', '1 y', '1 z', '2 x', '2 y', '2 z']]
    """
    if len(arg)<2:
        return arg

    return concat([[x+blank+y for x in arg[0] for y in arg[1]]]+arg[2:],blank)


def ExpandRE(optstr):

    """
    expand intervals and discrete values in 'optstr'
    and return list of strings

    Example:
    x=[1,4,1]  expands to  ['1', '2', '3', '4']
    x={10,20}  expands to  ['10', '20']
    """
    
    # all values from expanded RE
    values = []

    # search for interval
    interval_re = "\[([-+]?[0-9.]+),([-+]?[0-9.]+),([-+]?[0-9.]+)\]"
    m = re.search(interval_re, optstr)
    while m != None:
        # expand interval into an array of values
        if (m.group(0).find(".") != -1):
            # float values
            range = numpy.arange(float(m.group(1)), \
                                 float(m.group(2))+float(m.group(3)), \
                                 float(m.group(3)))
        else:
            range = numpy.arange(int(m.group(1)), \
                                 int(m.group(2))+int(m.group(3)), \
                                 int(m.group(3)))
            
        values += [str(x) for x in range]
        optstr = optstr.replace(m.group(0),"")
        m = re.search(interval_re, optstr)

    # search for discrete values given by curly braces {1,2,3} etc.
    discrete_re = "\{([,]?[-a-zA-Z0-9.+]+).*?\}"
    m = re.search(discrete_re, optstr)
    while m != None:
        # expand interval into an array of values
        for val in [x.strip(",") for x in m.groups()]:
            if val != None:
                values.append(val)
        optstr = optstr.replace(m.group(1),"",1)
        m = re.search(discrete_re, optstr)

    if values == []:
        # no regular expression found -->
        # return single value after '='
        m = re.search("=(.*)", optstr)
        if (m != None):
            return [m.group(1)]
        else:
            return [optstr]
    else:
        return values


def GetVarName(optstr):
    """
    extract 'x=' from a single option string like 'x=value'
    """
    m = re.search("^(.*)=", optstr)
    if m == None:
        return ''
    return m.group(1)


def ExpandOptionString(optstr):
    """
    expand symbols [] and {} in option string and return
    list of option strings

    Parameters:
    - optstr - option string of the form
    :var1={val1,val2,val3}[start,stop,step]:var2=[start,stop,step]:

    Return value:
    list of expanded option strings
    """
    substring  = [x for x in optstr.split(':') if x!='']
    exp_substr = [(GetVarName(x),ExpandRE(x)) for x in substring]

    # concat variable name and value from list
    touples = []
    for (x,y) in exp_substr:
        lst = []
        for yi in y:
            if x!='':
                lst.append(x+'='+yi)
            else:
                lst.append(yi)
        touples.append(lst)

    return concat(touples,':')[0]
