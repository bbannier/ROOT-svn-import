// the class representing all selection rules
#ifndef SELECTIONRULES_H
#define SELECTIONRULES_H

#include <list>
#include "BaseSelectionRule.h"
#include "ClassSelectionRule.h"
#include "VariableSelectionRule.h"
#include "clang/AST/Decl.h"


enum ESelectionFileTypes { // type of selection file
   kSelectionXMLFile,
   kLinkdefFile
};

class SelectionRules{

private:
   std::list<ClassSelectionRule> fClassSelectionRules;       // list of the clss selection rules
   std::list<FunctionSelectionRule> fFunctionSelectionRules; // list of the global functions selection rules
   std::list<VariableSelectionRule> fVariableSelectionRules; // list of the global variables selection rules
   std::list<EnumSelectionRule> fEnumSelectionRules;         // list of the enums selection rules

   ESelectionFileTypes fSelectionFileType;

   bool isDeep; // if --deep option passed from command line, this should be set to true
   bool hasFileNameRule; // if we have a file name rule, this should be set to true

   
public:
   SelectionRules(){}
   
   void addClassSelectionRule(const ClassSelectionRule& classSel);
   bool hasClassSelectionRules();
   const std::list<ClassSelectionRule>& getClassSelectionRules();
   
   void addFunctionSelectionRule(const FunctionSelectionRule& funcSel);
   bool hasFunctionSelectionRules();
   const std::list<FunctionSelectionRule>& getFunctionSelectionRules();
   
   void addVariableSelectionRule(const VariableSelectionRule& varSel);
   bool hasVariableSelectionRules();
   const std::list<VariableSelectionRule>& getVariableSelectionRules();
   
   void addEnumSelectionRule(const EnumSelectionRule& enumSel);
   bool hasEnumSelectionRules();
   const std::list<EnumSelectionRule>& getEnumSelectionRules();
   
   void printSelectionRules(); // print all selection rules
   
   void clearSelectionRules(); // clear all selection rules

   void setHasFileNameRule(bool file_rule);
   bool getHasFileNameRule();

   void setDeep(bool deep);
   bool getDeep();
   
   bool isDeclSelected(clang::Decl* D); // this is the method which is called from clr-scan and returns true if the Decl 
                                        // selected, false otherwise

   bool isClassSelected(clang::Decl* D, const std::string& qual_name); // is the class selected
   
   // is the global function, variable, enum selected - the behavior is different for linkdef.h and selection.xml - that's why
   // we have two functions
   bool isVarFunEnumSelected(clang::Decl* D, const std::string& kind, const std::string& qual_name);
   bool isLinkdefVarFunEnumSelected(clang::Decl* D, const std::string& kind, const std::string& qual_name);

   // is member (field, method, enum) selected; the behavior for linkdef.h methods is different   
   bool isMemberSelected(clang::Decl* D, const std::string& kind, const std::string& str_name);
   bool isLinkdefMethodSelected(clang::Decl* D, const std::string& kind, const std::string& qual_name);
   
   // returns true if the parent is class or struct
   bool isParentClass(clang::Decl* D);
   
   // the same but returns also the parent name and qualified name
   bool isParentClass(clang::Decl* D, std::string& parent_name, std::string& parent_qual_name);

   // returns the parent name and qualified name
   bool getParentName(clang::Decl* D, std::string& parent_name, std::string& parent_qual_name);
   

   //bool getParent(clang::Decl* D, clang::Decl* parent); - this method would have saved a lot of efforts but it crashes
   // and I didn't understand why

   // gets the name and qualified name of the Decl
   bool getDeclName(clang::Decl* D, std::string& name, std::string& qual_name);
   
   // gets the name of the source file where the Decl was declared
   bool GetDeclSourceFileName(clang::Decl* D, std::string& file_name);

   // gets the function prototype if the Decl (if it is global function or method)
   bool getFunctionPrototype(clang::Decl* D, std::string& prototype);

   bool isSelectionXMLFile();
   bool isLinkdefFile();
   void setSelectionFileType(ESelectionFileTypes fileType);

   // returns true if all selection rules are used at least once
   bool areAllSelectionRulesUsed();
};

#endif
