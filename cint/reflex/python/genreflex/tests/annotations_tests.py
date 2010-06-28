import os.path
import sys
sys.path.append(os.path.dirname(sys.path[0]))

import unittest
import annotations

CommentBlock = annotations.CommentBlock
Object = annotations.Object
Field = annotations.Field
List = annotations.List
Annotation = annotations.Annotation
Literal = annotations.Literal

class AnnotationsTests(unittest.TestCase):

    def setUp(self):
        pass

    def tearDown(self):
        pass

    def assertCommentBlock(self, text_lines, next_code_line, actual):
        self.assertEquals(text_lines, actual.text_lines)
        self.assertEquals(next_code_line, actual.next_code_line)

    def assertCommentBlocks(self, header_file, expectations):
        actuals = annotations.comments(annotations.file_lines(header_file))
        self.assertEquals(len(expectations), len(actuals))
        for i, (text_lines, next_code_line) in enumerate(expectations):
            self.assertCommentBlock(text_lines, next_code_line, actuals[i])

    def test_text_with_no_comment(self):
        self.assertCommentBlocks("NoComment.h", [])
    def test_text_with_single_C_comment(self):
        self.assertCommentBlocks("SingleCComment.h", [(["Here I am"],8)])
    def test_text_with_C_comment_with_trailing_spaces(self):
        self.assertCommentBlocks("TrailingSpacesComment.h", [(["Lost in the middle"],8)])
    def test_text_with_many_C_comments(self):
        self.assertCommentBlocks("ManyCComments.h", [(["namespace comment"],5)
                                                    ,(["class comment"],9)
                                                    ,(["method comment"],14)])
    def test_text_with_mulitlined_C_comments(self):
        self.assertCommentBlocks("MultilinedCComment.h", [(["SingleCComment class"
                                                           ,"This class is central to the design"],9)])
    def test_text_with_monolined_Cpp_comment(self):
        self.assertCommentBlocks("MonolinedCppComment.h", [(["Catch me if you can"],12)])
    def test_text_with_single_Cpp_comment(self):
        self.assertCommentBlocks("SingleCppComment.h", [(["SingleCppComment::greeting"
                                                         ,"This method is central to the design"],13)])
    def test_text_with_Cpp_comment_with_trailing_stars(self):
        self.assertCommentBlocks("TrailingStarsComment.h", [(["TrailingStarsComment class"
                                                             ,"Because I'm worth it !"
                                                             ,""],10)])
    def test_text_with_Cpp_comment_long_before_code(self):
        self.assertCommentBlocks("LongBeforeComment.h", [(["LongBeforeComment header"
                                                          ,"A masterpiece !"],12)])

    def test_declarations_from_comment_with_text(self):
        self.assertEquals([], annotations.declarations(["Comment without annotation"]))
    def test_declarations_from_comment_with_an_annotation(self):
        self.assertEquals(["MyAnnotation(Value=1)"], annotations.declarations(["@custom:MyAnnotation(Value=1)"]))
    def test_declarations_from_comment_with_text_and_an_annotation(self):
        self.assertEquals(["MyAnnotation()"], annotations.declarations(["Here starts my comment", "@custom:MyAnnotation()"]))
    def test_declarations_from_comment_with_text_and_two_annotations(self):
        self.assertEquals(["MyAnnotation()", "HisAnnotation()"], annotations.declarations(["Here starts my comment", "@custom:MyAnnotation()", "@custom:HisAnnotation()"]))
    def test_declarations_from_comment_with_a_multiline_annotation(self):
        self.assertEquals(["MyAnnotation(Value=\"hello\")"], annotations.declarations(["@custom:MyAnnotation(", "Value=\"hello\")"]))

    def test_tokens_from_simple_declaration(self):
        self.assertEquals(["MyAnnotation","(",")"], annotations.tokens("MyAnnotation()"))
    def test_tokens_from_declaration_with_namespace(self):
        self.assertEquals(["home::MyAnnotation","(",")"], annotations.tokens("home::MyAnnotation()"))
    def test_tokens_from_declaration_with_whitespaces(self):
        self.assertEquals(["YourAnnotation","(",")"], annotations.tokens("YourAnnotation ( )"))
    def test_tokens_from_declaration_with_a_value(self):
        self.assertEquals(["MyAnnotation","(","Five","=","5",")"], annotations.tokens("MyAnnotation(Five=5)"))
    def test_tokens_from_declaration_with_many_values(self):
        self.assertEquals(["MyAnnotation","(","Five","=","5",",","Six","=","6",")"], annotations.tokens("MyAnnotation(Five=5,Six=6)"))
    def test_tokens_from_declaration_with_a_string_value(self):
        self.assertEquals(["MyAnnotation","(","Name","=","\"Snoopy\"",")"], annotations.tokens("MyAnnotation(Name=\"Snoopy\")"))
    def test_tokens_from_declaration_with_an_escaped_string_value(self):
        self.assertEquals(["MyAnnotation","(","Quote","=",'"\\"Only the goods die young\\""',")"], annotations.tokens('MyAnnotation(Quote="\\"Only the goods die young\\"")'))
    def test_tokens_from_declaration_with_an_object_value(self):
        self.assertEquals(["MyAnnotation","(","Value","=","@","MyValue","(",")",")"], annotations.tokens("MyAnnotation(Value=@MyValue())"))
    def test_tokens_from_declaration_with_a_list_of_objects_value(self):
        self.assertEquals(["MyAnnotation","(","Value","=","{","@","MyValue","(",")",",","@","YourValue","(",")","}",")"], annotations.tokens("MyAnnotation(Value={@MyValue(),@YourValue()})"))
    def test_tokens_from_declaration_with_two_strings(self):
        self.assertEquals(["MyAnnotation","(","Dog","=",'"Snoopy"',",","Boy","=",'"Charly Brown"',")"], annotations.tokens('MyAnnotation(Dog="Snoopy",Boy="Charly Brown")'))
    def test_tokens_from_declaration_with_nested_objects_and_two_strings(self):
        self.assertEquals(['Credits','(','Author','=','"Philippe Bourgau"',',','Version','=','@','Version','(','Major','=','"0.9"',')',')'], annotations.tokens('Credits(Author="Philippe Bourgau",Version=@Version(Major="0.9"))'))
        
    def test_parse_a_simple_declaration(self):
        self.assertEquals(Object("MyAnnotation"), annotations.parse("MyAnnotation()"))
    def test_parse_a_declaration_with_a_value(self):
        self.assertEquals(Object("MyAnnotation",[Field("Five",Literal("5"))]), annotations.parse("MyAnnotation(Five=5)"))
    def test_parse_a_declaration_with_many_value(self):
        self.assertEquals(Object("MyAnnotation",[Field("Five",Literal("5")),Field("Six",Literal("\"VI\""))]), annotations.parse("MyAnnotation(Five=5,Six=\"VI\")"))
    def test_parse_a_declaration_with_an_object_value(self):
        self.assertEquals(Object("MyAnnotation",[Field("Value",Object("MyValue"))]), annotations.parse("MyAnnotation(Value=@MyValue())"))
    def test_parse_a_declaration_with_a_list_of_objects_value(self):
        self.assertEquals(Object("MyAnnotation",[Field("Value",List([Object("MyValue"),Object("YourValue")]))]), annotations.parse("MyAnnotation(Value={@MyValue(),@YourValue()})"))

    def test_end_to_end_annotation_parsing(self):
        self.assertEquals([Annotation(Object("Credits",[Field("Author",Literal("\"Philippe Bourgau\""))]), 11),
                           Annotation(Object("Serializable"), 11),
                           Annotation(Object("Getter"), 16)],
                          annotations.fromfile("ManyAnnotations.h"))
        

if __name__ == "__main__":
    #import sys;sys.argv = ['', 'Test.testName']
    unittest.main()
