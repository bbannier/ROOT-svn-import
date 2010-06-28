# 2010, All rights reserved.
#
# Permission to use, copy, modify, and distribute this software for any
# purpose is hereby granted without fee, provided that this copyright and
# permissions notice appear in all copies and derivatives.
#
# This software is provided "as is" without express or implied warranty.

import re

class Object():
    """AST for an object instanciated in an annotation.
    """

    def gettype(self):
        """Name of the class of the object.
        """
        return self.type
    def getfields(self):
        """Initialized fields and values.
        """
        return self.fields

    def collectInitCode(self, lines):
        """ Returns an expression that can be used later in the code to refer to this
        initialized value. Can add lines of initialization code if needed.
        """
        result = "object_"+str(len(lines))
        lines.append("  %s* %s = new %s();"%(self.gettype(), result, self.gettype()))
        for field in self.getfields():
            field.collectInitCode(result, lines)
          
        return result

    def __init__(self, type, fields=[]):
        self.type = type
        self.fields = fields

    def __repr__(self):
        fields = ""
        if 0 < len(self.fields):
            fields = ",["+",".join([repr(v) for v in self.fields])+"]"
        return "annotations.Object(\"%s\"%s)"%(self.type, fields)

    def __eq__(self, other):
        return (self.type == other.type) and (self.fields == other.fields)

class List():
    """AST for a vector of objects instanciated in an annotation.
    """

    def getitems(self):
        """ASTs for the objects in the vector.
        """
        return self.items

    def collectInitCode(self, listVar, lines):
        """ Adds lines of code to initialize an existing list.
        """
        for item in self.getitems():
            lines.append("  %s.push_back(%s);"%(listVar, item.collectInitCode(lines)))

    def __init__(self, items):
        self.items = items

    def __repr__(self):
        return "List("+",".join([repr(v) for v in self.items])+")"

    def __eq__(self, other):
        return self.items == other.items

class Field():
    """AST describing the initialization of the field of an object.
    """
    
    def getname(self):
        """Name of the field to initialize.
        """
        return self.name
    def getvalue(self):
        """Value to initialize the field with.
        """
        return self.value

    def collectInitCode(self, objectVar, lines):
        """ Adds lines of code to initialize this field for an existing object.
        """
        name = self.getname()
        value = self.getvalue()
        if isinstance(value, List):
            value.collectInitCode("%s->get%s()"%(objectVar, name), lines)
        else:
            lines.append("  %s->set%s(%s);"%(objectVar, name, value.collectInitCode(lines)))

    def __init__(self, name, value):
        self.name = name
        self.value = value

    def __repr__(self):
        return "annotations.Field(\"%s\",%s)"%(self.name,self.value)

    def __eq__(self, other):
        return (self.name == other.name) and (self.value == other.value)

class Literal():
    """AST describing a literal value.
    """
    
    def getvalue(self):
        """Value of the literal.
        """
        return self.value

    def collectInitCode(self, lines):
        """ Returns an expression that can be used later in the code to refer to this
        initialized value. Can add lines of initialization code if needed.
        """
        return self.value

    def __init__(self, value):
        self.value = value

    def __repr__(self):
        return "annotations.Literal(%s)"%repr(self.value)

    def __eq__(self, other):
        return self.value == other.value

class Annotation:
    """Objects representing a C++ annotations.
    """

    def getast(self):
        """Abstract synthax tree representing the annotation.
        """
        return self.ast
    def getcodeline(self):
        """Line of code to which this annotation is related.
        """
        return self.codeline
    
    def __init__(self, ast, codeline):
        self.ast = ast
        self.codeline = codeline

    def __repr__(self):
        return "annotations.Annotation(%s,%s)"%(self.ast,self.codeline)

    def __eq__(self, other):
        return (self.ast == other.ast) and (self.codeline == other.codeline)

def fromfile(file):
    """Reads a C++ file and creates the list of Annotation
    abstract synthax tree decribed in it.
    """

    all_lines = file_lines(file)
    comment_blocks = comments(all_lines)
    result = []
    for block in comment_blocks:
        decls = declarations(block.text_lines)
        for decl in decls:
            ast = parse(decl)
            result.append(Annotation(ast, block.next_code_line))
    return result


def file_lines(file):
    # reads a file and returns the lines from it
    f = open(file)
    try:
        return f.readlines()
    finally:
        f.close()

class CommentBlock:
    # a comment block, with its text and related code line
    
    def __init__(self, text_lines, next_code_line=-1):
        self.text_lines = text_lines
        self.next_code_line = next_code_line

# Comment block readers are responsible to extract the comment
# contained in a line of code.

class CCommentReader:
    # reader for C style comment blocks
    # simply returns the text after // in a line of code

    def lookin(self, line):
        if not line.startswith('//'):
            return None
    
        return line[2:].strip()

class CppCommentReader:
    # reader for C++ style comment blocks
    # keeps track of comments start and end to know
    # if a line of code is inside a comment or no

    def __init__(self):
        self.inside_comment = False

    def lookin(self, line):
        result = None

        if self.inside_comment:
            result = line
        elif line.startswith('/*'):
            result = line[2:]
            self.inside_comment = True

        if result and result.endswith('*/'):
            result = result[:-2]
            self.inside_comment = False

        if result:
            result = result.strip(" \t\r\n*")
            
        return result

def comments(lines):
    # extracts comment blocks from lines of code

    result = []

    c_comment = CCommentReader()
    cpp_comment = CppCommentReader()
    current_comment = None
    for i, l in enumerate(lines):
        line = l.strip()
        comment = c_comment.lookin(line) or cpp_comment.lookin(line)

        # be careful, "" is evaluated to False ...
        if None != comment:
            if not current_comment:
                current_comment = CommentBlock([])
                result.append(current_comment)

            current_comment.text_lines.append(comment)

        elif current_comment and line:
            current_comment.next_code_line = i+1
            current_comment = None

    return result

def declarations(comments):
    # list of annotation declarations (as strings) from lines of comment text

    def fold(result, next):
        if next.startswith("@custom:"):
            _prefix, _sep, declaration = next.partition("@custom:")
            return result + [declaration]
        elif [] != result:
            result[-1] += next
            return result
        else:
            return result

    return reduce(fold, comments, [])

# lexer regexps
expres = ["[a-zA-Z0-9:_]+",
          "\(",
          "\)",
          "=",
          ",",
          '"(?:(?:\\\\")|[^"])*"',
          "@",
          "{",
          "}"]
regexps = [re.compile("^(%s)(.*)$"%e) for e in expres]

def tokens(declaration):
    # returns a list of tokens from an annotation declaration string

    def firstMatch(declaration):
        for regex in regexps:
            match = regex.match(declaration)
            if match:
                return match
        return None

    result = []
    while 0 < len(declaration):
        match = firstMatch(declaration)
        if match:
            result.append(match.group(1))
            declaration = match.group(2).strip()
        else:
            print declaration+"\n"
            raise "invalid annotation declaration"

    return result

# parsing functions using a recursive descendant parser

def next(tokens):
    return tokens[-1]

def assertAst(expected, token):
    if expected != token:
        raise "unexpected token"

def collectionAst(tokens, endToken, itemAstFunction):
    result = []
    while next(tokens) != endToken:
        result.append(itemAstFunction(tokens))
        if next(tokens) == ",":
            tokens.pop()
    return result

def listAst(tokens):
    return List(collectionAst(tokens, "}", valueAst))

def valueAst(tokens):
    tok = tokens.pop()
    if tok == "@":
        return objectAst(tokens)
    elif tok == "{":
        result = listAst(tokens)
        assertAst("}",tokens.pop())
        return result
    else:
        return Literal(tok)

def fieldAst(tokens):
    name = tokens.pop()
    assertAst("=",tokens.pop())
    value = valueAst(tokens)
    return Field(name, value)

def fieldsAst(tokens):
    return collectionAst(tokens, ")", fieldAst)

def objectAst(tokens):
    name = tokens.pop()
    assertAst("(",tokens.pop())
    values = fieldsAst(tokens)
    assertAst(")",tokens.pop())
    return Object(name, values)

def ast(tokens):
    return objectAst(tokens)

def parse(declaration):
    toks = tokens(declaration)
    toks.reverse()
    return ast(toks)



# TODO:

#  trouver pourquoi certains tests echouent en local concernant les casts lancer tout le build en local avec ma modif

#  call annotation generation code from genreflex
#  add annotation code to Reflex
#  generate C++ code

# OPEN QUESTIONS:
#  how to write initialization code: full method calls, setNAME(VALUE), NAME=VALUE ?

