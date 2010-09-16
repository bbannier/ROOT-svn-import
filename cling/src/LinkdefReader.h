// Linkdef.h parsing class
//#include <iostream>
#include <fstream>
#include <vector>
#include "SelectionRules.h"

class LinkdefReader {
 private:
   int line; // lines count - for error messages

 public:
 LinkdefReader():line(1) {}

 private:

   enum EPragmaNames { // the processed pragma attributes
      kAll,
      kNestedclasses,
      kDefinedIn,
      kGlobal,
      kFunction,
      kEnum,
      kClass,
      kUnion,
      kStruct,
      kOperators,
      kUnknown
  };

   enum ECppNames{ // the processes pre-processor directives
      kPragma,
      kIfdef,
      kEndif,
      kIf,
      kElse,
      kUnrecognized
   };
   
   // used to create string to tag kind association to use in switch constructions
   static std::map<std::string, EPragmaNames> fgMapPragmaNames;
   static std::map<std::string, ECppNames> fgMapCppNames;
   
   
public:
   static void PopulatePragmaMap();
   static void PopulateCppMap();
   
   bool CPPHandler(std::ifstream& file, SelectionRules& sr); // this is the main parsing method
   void PrintAllTokens(std::ifstream& file); // for debugging purposes
   
private:
   void TrimChars(std::string& out, const std::string& chars); // helper function trims the chars passed as second argument
   bool StartsWithPound(std::ifstream& file);
   bool PragmaParser(std::ifstream& file, SelectionRules& sr); // parses every pragma statement
   bool RemoveComment(std::ifstream& file);                    // rmoves comments from the pragma statements
   bool GetNextToken(std::ifstream& file, std::string& token, bool str); // gets next token from a pragma statement
   bool GetFirstToken(std::ifstream& file, std::string& token);   // detects the begining of a statement and returns the first token (#pragma, #if, ...)
   bool IsLastSemiColon(std::string& str); 
   bool ProcessFunctionPrototype(std::string& proto, bool& name); // transforms the function prototypes to a more unified form
   bool ProcessOperators(std::string& pattern); // transforms the operators statement to the suitable function pattern

   bool IsPatternRule(const std::string& rule_token); // is it name or pattern
};
