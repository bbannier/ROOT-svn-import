#include "SelectionRules.h"
#include <iostream>
#include "TString.h"
#include "clang/Basic/SourceLocation.h"
#include "clang/Basic/SourceManager.h"
#include "clang/AST/ASTContext.h"

void SelectionRules::addClassSelectionRule(const ClassSelectionRule& classSel)
{
  fClassSelectionRules.push_back(classSel);
}

bool SelectionRules::hasClassSelectionRules()
{
  return !fClassSelectionRules.empty();
}

const std::list<ClassSelectionRule>& SelectionRules::getClassSelectionRules()
{
  return fClassSelectionRules;
}

void SelectionRules::addFunctionSelectionRule(const FunctionSelectionRule& funcSel)
{
  fFunctionSelectionRules.push_back(funcSel);
}

bool SelectionRules::hasFunctionSelectionRules()
{
  return !fFunctionSelectionRules.empty();
}

const std::list<FunctionSelectionRule>& SelectionRules::getFunctionSelectionRules()
{
  return fFunctionSelectionRules;
}

void SelectionRules::addVariableSelectionRule(const VariableSelectionRule& varSel)
{
  fVariableSelectionRules.push_back(varSel);
}

bool SelectionRules::hasVariableSelectionRules()
{
  return !fVariableSelectionRules.empty();
}

const std::list<VariableSelectionRule>& SelectionRules::getVariableSelectionRules()
{
  return fVariableSelectionRules;
}

void SelectionRules::addEnumSelectionRule(const EnumSelectionRule& enumSel)
{
  fEnumSelectionRules.push_back(enumSel);
}

bool SelectionRules::hasEnumSelectionRules()
{
  return !fEnumSelectionRules.empty();
}

const std::list<EnumSelectionRule>& SelectionRules::getEnumSelectionRules()
{
  return fEnumSelectionRules;
}

void SelectionRules::printSelectionRules()
{
  std::cout<<"Printing Selection Rules:"<<std::endl;
  if (!fClassSelectionRules.empty()) {
    int i = 0;
    for(std::list<ClassSelectionRule>::iterator it = fClassSelectionRules.begin(); 
	it != fClassSelectionRules.end(); ++it, ++i) {
      std::cout<<"\tClass sel rule "<<i<<":"<<std::endl;
      std::cout<<"\t\tSelected: ";
      switch(it->getSelected()){
      case kYes: std::cout<<"Yes"<<std::endl;
	break;
      case kNo: std::cout<<"No"<<std::endl;
	break;
      case kDontCare: std::cout<<"Don't Care"<<std::endl;
	break;
      default: std::cout<<"Unspecified"<<std::endl;
      }
      std::cout<<"\t\tAttributes: "<<std::endl;
      it->printAttributes(2);
      
      if (it->hasFieldSelectionRules()) {
	//std::cout<<"\t\tHas field entries"<<std::endl;
	std::list<VariableSelectionRule> fields = it->getFieldSelectionRules();
	std::list<VariableSelectionRule>::iterator fit = fields.begin();
	int j = 0;
	
	for (; fit != fields.end(); ++fit, ++j) 
	{
	  std::cout<<"\t\tField "<<j<<":"<<std::endl;
	  std::cout<<"\t\tSelected: ";
	  switch(fit->getSelected()){
	  case kYes: std::cout<<"Yes"<<std::endl;
	    break;
	  case kNo: std::cout<<"No"<<std::endl;
	    break;
	  case kDontCare: std::cout<<"Don't Care"<<std::endl;
	    break;
	  default: std::cout<<"Unspecified"<<std::endl;
	  }
	  fit->printAttributes(3);
	}
      } 
      else {
	std::cout<<"\t\tNo field sel rules"<<std::endl;
      }
      if (it->hasMethodSelectionRules()) {
	//std::cout<<"\t\tHas method entries"<<std::endl;
	std::list<FunctionSelectionRule> methods = it->getMethodSelectionRules();
	std::list<FunctionSelectionRule>::iterator mit = methods.begin();
	int k = 0;

	for (; mit != methods.end(); ++mit, ++k) 
	{
	  std::cout<<"\t\tMethod "<<k<<":"<<std::endl;
	  std::cout<<"\t\tSelected: ";
	  switch(mit->getSelected()){
	  case kYes: std::cout<<"Yes"<<std::endl;
	    break;
	  case kNo: std::cout<<"No"<<std::endl;
	    break;
	  case kDontCare: std::cout<<"Don't Care"<<std::endl;
	    break;
	  default: std::cout<<"Unspecified"<<std::endl;
	  }
	  mit->printAttributes(3);
	}
      }
      else {
	std::cout<<"\t\tNo method sel rules"<<std::endl;
      }
    }
  }
  else { 
    std::cout<<"\tNo Class Selection Rules"<<std::endl;
  }

  if (!fFunctionSelectionRules.empty()) {
    //std::cout<<""<<std::endl;
    std::list<FunctionSelectionRule>::iterator it2;
    int i = 0;

    for (it2 = fFunctionSelectionRules.begin(); it2 != fFunctionSelectionRules.end(); ++it2, ++i) {
      std::cout<<"\tFunction sel rule "<<i<<":"<<std::endl;
      std::cout<<"\t\tSelected: ";
      switch(it2->getSelected()){
      case kYes: std::cout<<"Yes"<<std::endl;
	break;
      case kNo: std::cout<<"No"<<std::endl;
	break;
      case kDontCare: std::cout<<"Don't Care"<<std::endl;
	break;
      default: std::cout<<"Unspecified"<<std::endl;
      }
      it2->printAttributes(2);
    }
  }
  else {
    std::cout<<"\tNo function sel rules"<<std::endl;
  }

  if (!fVariableSelectionRules.empty()) {
    std::list<VariableSelectionRule>::iterator it3;
    int i = 0;

    for (it3 = fVariableSelectionRules.begin(); it3 != fVariableSelectionRules.end(); ++it3, ++i) {
      std::cout<<"\tVariable sel rule "<<i<<":"<<std::endl;
      std::cout<<"\t\tSelected: ";
      switch(it3->getSelected()){
      case kYes: std::cout<<"Yes"<<std::endl;
	break;
      case kNo: std::cout<<"No"<<std::endl;
	break;
      case kDontCare: std::cout<<"Don't Care"<<std::endl;
	break;
      default: std::cout<<"Unspecified"<<std::endl;
      }
      it3->printAttributes(2);
    }
  }
  else {
    std::cout<<"\tNo variable sel rules"<<std::endl;
  }

  if (!fEnumSelectionRules.empty()) {
    std::list<EnumSelectionRule>::iterator it4;
    int i = 0;

    for (it4 = fEnumSelectionRules.begin(); it4 != fEnumSelectionRules.end(); ++it4, ++i) {
      std::cout<<"\tEnum sel rule "<<i<<":"<<std::endl;
      std::cout<<"\t\tSelected: ";
      switch(it4->getSelected()){
      case kYes: std::cout<<"Yes"<<std::endl;
	break;
      case kNo: std::cout<<"No"<<std::endl;
	break;
      case kDontCare: std::cout<<"Don't Care"<<std::endl;
	break;
      default: std::cout<<"Unspecified"<<std::endl;
      }
      it4->printAttributes(2);
    }
  }
  else {
    std::cout<<"\tNo enum sel rules"<<std::endl;
  }
}

void SelectionRules::clearSelectionRules()
{
  if (!fClassSelectionRules.empty()) {
    fClassSelectionRules.clear();
  }
  if (!fFunctionSelectionRules.empty()) {
    fFunctionSelectionRules.clear();
  }
  if (!fVariableSelectionRules.empty()) {
    fVariableSelectionRules.clear();
  }
  if (!fEnumSelectionRules.empty()) {
    fEnumSelectionRules.clear();
  }
}

void SelectionRules::setHasFileNameRule(bool file_rule)
{
   hasFileNameRule = file_rule;
}

bool SelectionRules::getHasFileNameRule()
{
   if (hasFileNameRule) return true;
   else return false;
}

void SelectionRules::setDeep(bool deep)
{
   isDeep = deep;
   if (isDeep) {
      ClassSelectionRule csr;
      csr.setAttributeValue("pattern", "*");
      csr.setSelected(kYes);
      addClassSelectionRule(csr);

      ClassSelectionRule csr2;
      csr2.setAttributeValue("pattern", "*::*");
      csr2.setSelected(kYes);
      addClassSelectionRule(csr2);
      

      // Should I disable the built-in (automatically generated) structs/classes?
      ClassSelectionRule csr3;
      csr3.setAttributeValue("pattern", "__va_*"); // <built-in> 
      csr3.setSelected(kNo);
      addClassSelectionRule(csr3);
      //hasFileNameRule = true;

      //setSelectionXMLFile(true);
   }
}

bool SelectionRules::getDeep()
{
   if (isDeep) return true;
   else return false;
}


bool SelectionRules::isDeclSelected(clang::Decl *D)
{  
   std::string str_name;   // name of the Decl
   std::string kind;       // kind of the Decl
   std::string qual_name;  // fully qualified name of the Decl
   
   if (!D) {
      return false;
   }

   kind = D->getDeclKindName();

   getDeclName(D, str_name, qual_name);
      
   //DEBUG std::cout<<"\nTESTING - Decl "<<str_name<<" (qual_name: "<<qual_name<<") of kind "<<kind;
   
   if (kind == "CXXRecord") { // structs, unions and classes are all CXXRecords
      return isClassSelected(D, qual_name);
   }

   if (kind == "Var" || kind == "Function") {
      if (!isLinkdefFile())
         return isVarFunEnumSelected(D, kind, qual_name);
      else
         return isLinkdefVarFunEnumSelected(D, kind, qual_name);
   } 

   if (kind == "Enum"){

      // Enum is part of a class
      if (isParentClass(D)) {
         if (!isMemberSelected(D, kind, str_name)) // if the parent class is deselected, we could still select the enum
            return isVarFunEnumSelected(D, kind, qual_name);
         else           // if the parent class is selected so are all nested enums
            return true;
      }

      // Enum is not part of a class
      else {
         if (isLinkdefFile())
            return isLinkdefVarFunEnumSelected(D, kind, qual_name);
         return isVarFunEnumSelected(D, kind, qual_name);
      }
   }

   if (kind == "CXXMethod" || kind == "CXXConstructor" || kind == "CXXDestructor" || kind == "Field") {
      /* DEBUG
        if(kind != "Field"){
         std::string proto;
         if (getFunctionPrototype(D,proto))
	    std::cout<<"\n\tFunction prototype: "<<str_name + proto;
         else 
	    std::cout<<"Error in prototype formation"; 
      }
      */
      
      if (isLinkdefFile() && kind != "Field") {
         return isLinkdefMethodSelected(D, kind, qual_name);
      }
      return isMemberSelected(D, kind, str_name);
   }
      
   return true;
}

bool SelectionRules::getDeclName(clang::Decl* D, std::string& name, std::string& qual_name)
{
   clang::NamedDecl* N = dyn_cast<clang::NamedDecl> (D);

   if (N) {
      // the identifier is NULL for some special methods like constructors, destructors and operators
      if (N->getIdentifier()) { 
         name = N->getNameAsString();
      }
      else if (N->isCXXClassMember()) { // for constructors, destructors, operator=, etc. methods 
         name =  N->getNameAsString(); // we use this (unefficient) method to get the name in that case 
      }
      qual_name = N->getQualifiedNameAsString();
      return true;
   }
   else {
      return false;
   }  
}

bool SelectionRules::GetDeclSourceFileName(clang::Decl* D, std::string& file_name)
{
   clang::SourceLocation SL = D->getLocation();
   clang::ASTContext& ctx = D->getASTContext();
   clang::SourceManager& SM = ctx.getSourceManager();

   if (SL.isValid() && SL.isFileID()) {
      clang::PresumedLoc PLoc = SM.getPresumedLoc(SL);
      file_name = PLoc.getFilename();
      return true;
   }
   else {
      file_name = "invalid";
      return false;
   }   
}



bool SelectionRules::getFunctionPrototype(clang::Decl* D, std::string& prototype) {
   if (!D) {
      return false;
   }

   clang::FunctionDecl* F = dyn_cast<clang::FunctionDecl> (D); // cast the Decl to FunctionDecl
 
   if (F) {

   prototype = "";

   // iterate through all the function parameters
   for (clang::FunctionDecl::param_iterator I = F->param_begin(), E = F->param_end(); I != E; ++I) {
      clang::ParmVarDecl* P = *I;

      if (prototype != "")
         prototype += ",";

      std::string type = P->getType().getAsString();

      // pointers are returned in the form "int *" and I need them in the form "int*"
      if (type.at(type.length()-1) == '*') {
	 type.at(type.length()-2) = '*';
	 type.erase(type.length()-1);
      }
      prototype += type;
   }

   prototype = "(" + prototype + ")";
   return true;
   }
   else {
      std::cout<<"Warning - can't convert Decl to FunctionDecl"<<std::endl;
      return false;
   }
}


bool SelectionRules::isParentClass(clang::Decl* D)
{
   clang::DeclContext *ctx = D->getDeclContext();

   if (ctx->isRecord()){
      //DEBUG std::cout<<"\n\tDeclContext is Record";
      clang::Decl *parent = dyn_cast<clang::Decl> (ctx);
      if (!parent) {
         return false;
      }
      else {
         //TagDecl has methods to understand of what kind is the Decl - class, struct or union
         clang::TagDecl* T = dyn_cast<clang::TagDecl> (parent); 
         
         if (T) {
            if (T->isClass()||T->isStruct()) { 
               return true;
            }
            else {
               return false;
            }
         }
         else {
            return false;
         }
      }
   }
   else {
      return false;
   }
}


bool SelectionRules::isParentClass(clang::Decl* D, std::string& parent_name, std::string& parent_qual_name)
{
   clang::DeclContext *ctx = D->getDeclContext();

   if (ctx->isRecord()){
      //DEBUG std::cout<<"\n\tDeclContext is Record";
      clang::Decl *parent = dyn_cast<clang::Decl> (ctx);
      if (!parent) {
         return false;
      }
      else {
         //TagDecl has methods to understand of what kind is the Decl
         clang::TagDecl* T = dyn_cast<clang::TagDecl> (parent); 
         
         if (T) {
            if (T->isClass()|| T->isStruct()) { 
               getDeclName(parent, parent_name, parent_qual_name);
               return true;
            }
            else {
               return false;
            }
         }
         else {
            return false;
         }
      }
   }
   else {
      return false;
   }
}

bool SelectionRules::getParentName(clang::Decl* D, std::string& parent_name, std::string& parent_qual_name)
{
   clang::DeclContext *ctx = D->getDeclContext();

   if (ctx->isRecord()){
      //DEBUG std::cout<<"\n\tDeclContext is Record";
      clang::Decl *parent = dyn_cast<clang::Decl> (ctx);
      if (!parent) {
         return false;
      }
      else {
         getDeclName(parent, parent_name, parent_qual_name);
         return true;
      }
   }
   else {
      return false;
   }
}

/* This is the method that crashes
bool SelectionRules::getParent(clang::Decl* D, clang::Decl* parent)
{
   clang::DeclContext *ctx = D->getDeclContext();

   if (ctx->isRecord()){
      //DEBUG std::cout<<"\n\tDeclContext is Record";
      parent = dyn_cast<clang::Decl> (ctx);
      if (!parent) {
         return false;
      }
      else {
         return true;
      }
   }
   else return false;
}
*/


// isClassSelected checks if a class is selected or not. Thre is a difference between the
// behaviour of rootcint and genreflex especially with regard to class pattern processing.
// In genreflex if we have <class pattern = "*" /> this will select all the classes 
// (and structs) found in the header file. In rootcint if we have something similar, i.e.
// #pragma link C++ class *, we will select only the outer classes - for the nested
// classes we have to specifie #pragma link C++ class *::*. And yet this is only valid
// for one level of nesting - if you need it for many levels of nesting, you will 
// probably have to implement it yourself.
// Here the idea is the following - we traverse the list of class selection rules.
// For every class we check do we have a class selection rule. We use here the
// method isSelected() (defined in BaseSelectionRule.cxx). This method returns true
// only if we have class selection rule which says "Select". Otherwise it returns 
// false - in which case we have to check wether we found a class selection rule
// which says "Veto" (noName = false and don't Care = false; OR noName = false and
// don't Care = true and we don't have neither method nor field selection rules - 
// which is for the selection.xml file case). If noName is true than we just continue - 
// this means that the current class selection rule isn't applicable for this class.

bool SelectionRules::isClassSelected(clang::Decl* D, const std::string& qual_name)
{
   clang::TagDecl* T;
   try {
    T = dyn_cast<clang::TagDecl> (D); //TagDecl has methods to understand of what kind is the Decl
   }
   catch (std::exception& e) {
      return false;
   }
   if (T) {
      if (isLinkdefFile() || T->isClass() || T->isStruct()) {
         std::string file_name;
         if (getHasFileNameRule()){
	   if (!GetDeclSourceFileName(D, file_name)){
               // DEBUG   std::cout<<"Warning - can't obtain source file name "<<std::endl;
               // DEBUG else std::cout<<"Source file name: "<<file_name<<std::endl;
	   }
         }
         int fYes = 0;
         int fImplNo = 0;
         bool explicit_Yes = false;
         int fFileNo =0;
         bool file;

         std::list<ClassSelectionRule>::iterator it = fClassSelectionRules.begin();
         // iterate through all class selection rles
         for(; it != fClassSelectionRules.end(); ++it) {
            bool dontC, noName;
            bool yes;
            
            if (isLinkdefFile()){
               yes = it->isSelected(qual_name, "", file_name, dontC, noName, file, true);
            }
            else {
               yes = it->isSelected(qual_name, "", file_name, dontC, noName, file, false);
            }
            if (yes) {
               ++fYes;
               if (isLinkdefFile()){
                  // rootcint prefers explicit rules over pattern rules
                  if (it->hasAttributeWithName("name")) {
                     std::string name_value;
                     it->getAttributeValue("name", name_value);
                     if (name_value == qual_name) explicit_Yes = true;
                  }
                  if (it->hasAttributeWithName("pattern")) {
                     std::string pattern_value;
                     it->getAttributeValue("pattern", pattern_value);
                     if (pattern_value != "*" && pattern_value != "*::*") explicit_Yes = true;
                  }
               }
               //else std::cout<<"\tYes returned"<<std::endl;
            }
            else if (!noName && !dontC) { // = kNo (noName = false <=> we have named rule for this class)
               // dontC = false <=> we are not in the exclusion part (for genreflex)

               // DEBUG std::cout<<"\tisSelected() returned false and !noName and !DontC"<<std::endl;

               if (!isLinkdefFile()) {
                  // in genreflex - we could explicitly select classes from other source files
                  if (file) ++fFileNo; // if we have veto because of class defined in other source file -> implicit No
                  else {
                     std::cout<<"\tNo returned"<<std::endl;
                     return false; // explicit No returned
                  }
               }
               if (it->hasAttributeWithName("pattern")) { //this is for the Linkdef selection
                  std::string pattern_value;
                  it->getAttributeValue("pattern", pattern_value);
                  if (pattern_value == "*" || pattern_value == "*::*") ++fImplNo;
                  else 
                     return false;
               }
               else
                  return false;
            }
            else if (dontC && !(it->hasMethodSelectionRules()) && !(it->hasFieldSelectionRules())) {
               // DEBUG std::cout<<"\tisSelected() returned false and dontC and it has no field and method rules"<<std::endl;
               std::cout<<"Empty dontC returned = No"<<std::endl;
               return false;
            }
         }  
         if (isLinkdefFile()) {
            // for rootcint explicit (name) Yes is stronger than implicit (pattern) No which is stronger than implicit (pattern) Yes
            std::cout<<"\n\tfYes = "<<fYes<<", fImplNo = "<<fImplNo<<std::endl;
            if (explicit_Yes) return true;
            else if (fImplNo > 0) return false;
            else return true;
         }
         else {                                 
            // for genreflex explicit Yes is stronger than implicit file No
            std::cout<<"\n\tfYes = "<<fYes<<", fFileNo = "<<fFileNo<<std::endl;
            if (fYes > 0) 
               return true;
            else 
               return false;
         }         
      }
      else { // Union (for genreflex)
         return false;
      }
   }
   else {
      std::cout<<"\n\tCouldn't cast Decl to TagDecl";
      return false;
   }
   
}


bool SelectionRules::isVarFunEnumSelected(clang::Decl* D, const std::string& kind, const std::string& qual_name)
{
   std::list<VariableSelectionRule>::iterator it;
   std::list<VariableSelectionRule>::iterator it_end;
   std::string prototype;
   
   if (kind == "Var") {
      it = fVariableSelectionRules.begin();
      it_end = fVariableSelectionRules.end();
   }
   else if (kind == "Function") {
      getFunctionPrototype(D, prototype);
      prototype = qual_name + prototype;
      std::cout<<"\tIn isVarFunEnumSelected()"<<prototype<<std::endl;
      it = fFunctionSelectionRules.begin();
      it_end = fFunctionSelectionRules.end();
   }
   else {
      it = fEnumSelectionRules.begin();
      it_end = fEnumSelectionRules.end();
   }

   std::string file_name;
   if (getHasFileNameRule()){
      if (GetDeclSourceFileName(D, file_name))
         std::cout<<"\tSource file name: "<<file_name<<std::endl;
   }
   
   int fYes = 0;
   bool d, n;
   bool selected;
   bool file;
   
   // iterate through all the rules 
   // we call this method only for genrefex variables, functions and enums - it is simpler than the class case:
   // if we have No - it is veto even if we have explicit yes as well
   for(; it != it_end; ++it) {
      if (kind == "Var") selected = it->isSelected(qual_name, "", file_name, d, n, file, false);
      else selected = it->isSelected(qual_name, prototype, file_name, d, n, file, false);
      if (selected) {
         ++fYes;
      }
      //if (it->isSelected(qual_name, d, n)) ++fYes;
      else if (!n) {
         return false;
      }
   }
   // DEBUG std::cout<<"\n\tfYes = "<<fYes;

      if (fYes == 0) {
         //DEBUG std::cout<<" (fNo = "<<fNo<<" fYes = "<<fYes<<") Returning false";
         return false;
      }
      else return true;
}


bool SelectionRules::isLinkdefVarFunEnumSelected(clang::Decl* D, const std::string& kind, const std::string& qual_name)
{
   std::list<VariableSelectionRule>::iterator it;
   std::list<VariableSelectionRule>::iterator it_end;
   std::string prototype;
   
   if (kind == "Var") {
      it = fVariableSelectionRules.begin();
      it_end = fVariableSelectionRules.end();
   }
   else if (kind == "Function") {
      getFunctionPrototype(D, prototype);
      prototype = qual_name + prototype;
      it = fFunctionSelectionRules.begin();
      it_end = fFunctionSelectionRules.end();
   }
   else {
      it = fEnumSelectionRules.begin();
      it_end = fEnumSelectionRules.end();
   }

   std::string file_name;
   if (getHasFileNameRule()){
      if (GetDeclSourceFileName(D, file_name))
         std::cout<<"\tSource file name: "<<file_name<<std::endl;
   }
   
   bool d, n;
   bool selected;
   int fYes = 0, fImplNo = 0;
   bool explicit_Yes = false;
   bool file;
   
   for(; it != it_end; ++it) {
      if (kind == "Var") selected = it->isSelected(qual_name, "", file_name, d, n, file, false);
      else selected = it->isSelected(qual_name, prototype, file_name, d, n, file, false);

      if(selected) {
         // explicit rules are with stronger priority in rootcint
         if (isLinkdefFile()){
            if (it->hasAttributeWithName("name")) {
               std::string name_value;
               it->getAttributeValue("name", name_value);
               if (name_value == qual_name) explicit_Yes = true;
            }
            if (it->hasAttributeWithName("pattern")) {
               std::string pattern_value;
               it->getAttributeValue("pattern", pattern_value);
               if (pattern_value != "*" && pattern_value != "*::*") explicit_Yes = true;
            }
         }
      }
      else if (!n) {
         if (!isLinkdefFile()) return false;
         else {
            if (it->hasAttributeWithName("pattern")) {
               std::string pattern_value;
               it->getAttributeValue("pattern", pattern_value);
               if (pattern_value == "*" || pattern_value == "*::*") ++fImplNo;
               else 
                  return false;
            }
            else
               return false;
         }
      }
   }

   if (isLinkdefFile()) {
      std::cout<<"\n\tfYes = "<<fYes<<", fImplNo = "<<fImplNo<<std::endl;
      if (explicit_Yes) return true;
      else if (fImplNo > 0) return false;
      else return true;
   }
   else{
      if (fYes == 0) {
         //DEBUG std::cout<<" (fNo = "<<fNo<<" fYes = "<<fYes<<") Returning false";
         return false;
      }
      else return true;
   }
}


// In rootcint we could select and deselect methods independantly of the class/struct/union rules
// That's why we first have to check the explicit rules for the functions - to see if there
// is rule corresponding to our method.
// Which is more - if we have (and we can have) a pattern for the parent class, than a pattern for the 
// nested class, than a pattern for certain methods in the nested class, than a rule for a 
// method (name or prototype) in the nested class - the valid rule is the last one.
// This is true irrespective of the rules (select/deselect). This is not the case for genreflex -
// in genreflex if there is a pattern deselecting something even if we have an explicit rule
// to select it, it still will not be selected.
// This method (isLinkdefMethodSelected()) might be incomplete (but I didn't have the time to think
// of anything better)
 // 

bool SelectionRules::isLinkdefMethodSelected(clang::Decl* D, const std::string& kind, const std::string& qual_name)
{
   std::list<FunctionSelectionRule>::iterator it = fFunctionSelectionRules.begin();
   std::list<FunctionSelectionRule>::iterator it_end = fFunctionSelectionRules.end();
   std::string prototype;

   getFunctionPrototype(D, prototype);
   prototype = qual_name + prototype;
   std::cout<<"\tFunction prototype = "<<prototype<<std::endl;

   int expl_Yes = 0, impl_r_Yes = 0, impl_rr_Yes = 0;
   int impl_r_No = 0, impl_rr_No = 0;
   bool d, n;
   bool selected;
   bool explicit_r = false, implicit_r = false, implicit_rr = false;
   bool file;
   
   if (kind == "CXXMethod"){
      // we first chack the explicit rules for the method (in case of constructors and destructors we check the parent)
      for(; it != it_end; ++it) {
         selected = it->isSelected(qual_name, prototype, "", d, n, file, false);
         
         if (selected || !n){
            // here I should implement my implicit/explicit thing
            // I have included two levels of implicitness - "A::get_*" is stronger than "*"
            if (it->hasAttributeWithName("name") || it->hasAttributeWithName("proto_name")) {
               explicit_r = true;
               if (selected) ++expl_Yes;
               else {
                  std::cout<<"\tExplicit rule kNo found"<<std::endl;
                  return false; // == explicit kNo
                  
               }
            }
            if (it->hasAttributeWithName("pattern")) {
               std::string pat_value;
               it->getAttributeValue("pattern", pat_value);
               
               if (pat_value == "*") continue; // we discard the global selection rules
               
               std::string par_name, par_qual_name;
               getParentName(D, par_name, par_qual_name);
               std::string par_pat = par_qual_name + "::*";
               
               if (pat_value == par_pat) {
                  implicit_rr = true;
                  if (selected) {
                     std::cout<<"Implicit_rr rule ("<<pat_value<<"), selected = "<<selected<<std::endl;
                     ++impl_rr_Yes;
                  }
                  else {
                     std::cout<<"Implicit_rr rule ("<<pat_value<<"), selected = "<<selected<<std::endl;
                     ++impl_rr_No;
                  }
               }
               else {
                  implicit_r = true;
                  if (selected) {
                     std::cout<<"Implicit_r rule ("<<pat_value<<"), selected = "<<selected<<std::endl;
                     ++impl_r_Yes;
                  }
                  else {
                     std::cout<<"Implicit_r rule ("<<pat_value<<"), selected = "<<selected<<std::endl;
                     ++impl_r_No;
                  }
               }
            }
         }
      } 
   }
   if (explicit_r /*&& expl_Yes > 0*/){
      std::cout<<"\tExplicit rule kYes found"<<std::endl;
      return true; // if we have excplicit kYes
   }
   else if (implicit_rr) {
      if (impl_rr_No > 0) {
         std::cout<<"\tImplicit_rr rule kNo found"<<std::endl;
         return false;
      }
      else {
         std::cout<<"\tImplicit_rr rule kYes found"<<std::endl;
         return true;
      }
   }
   else if (implicit_r) {
      if (impl_r_No > 0) {
         std::cout<<"\tImplicit_r rule kNo found"<<std::endl;
         return false;
      }
      else {
         std::cout<<"\tImplicit_r rule kYes found"<<std::endl;
         return true;
      }
   }
   else {
      std::cout<<"\tChecking parent class rules"<<std::endl;
      // check parent

      
      std::string parent_name, parent_qual_name;
      if (!getParentName(D, parent_name, parent_qual_name)) return false;
      
      std::string file_name;
      if (getHasFileNameRule()){
	if (GetDeclSourceFileName(D, file_name)){
            //std::cout<<"\tSource file name: "<<file_name<<std::endl;
	  }
      }

      int fYes = 0, fImplNo = 0;
      bool dontC, noName;
      bool explicit_Yes = false;

      // the same as with isClass selected
      // I wanted to use getParentDecl and then to pass i sto isClassSelected because I didn't wanted to repeat 
      // code but than getParentDecl crashes (or returns non-sence Decl) for the built-in structs (__va_*)
      std::list<ClassSelectionRule>::iterator it = fClassSelectionRules.begin();
      for(; it != fClassSelectionRules.end(); ++it) {
         bool yes;
         yes = it->isSelected(parent_qual_name, "", file_name, dontC, noName, file, true); // == kYes
  
         if (yes) {
            ++fYes;

            if (it->hasAttributeWithName("name")) {
               std::string name_value;
               it->getAttributeValue("name", name_value);
               if (name_value == parent_qual_name) explicit_Yes = true;
            }
            if (it->hasAttributeWithName("pattern")) {
               std::string pattern_value;
               it->getAttributeValue("pattern", pattern_value);
               if (pattern_value != "*" && pattern_value != "*::*") explicit_Yes = true;
            }
         }
         else if (!noName) { // == kNo

            if (it->hasAttributeWithName("pattern")) {
               std::string pattern_value;
               it->getAttributeValue("pattern", pattern_value);
               if (pattern_value == "*" || pattern_value == "*::*") ++fImplNo;
               else 
                  return false;
            }
            else
               return false;
         }
      }

      std::cout<<"\n\tfYes = "<<fYes<<", fImplNo = "<<fImplNo<<std::endl;
      if (explicit_Yes) {
         std::cout<<"\tReturning Yes"<<std::endl;
         return true;
      }
      else if (fImplNo > 0) {
         std::cout<<"\tReturning No"<<std::endl;
         return false;
      }
      else {
         std::cout<<"\tReturning Yes"<<std::endl;
         return true;
      }
   }

   return false; 

}

bool SelectionRules::isMemberSelected(clang::Decl* D, const std::string& kind, const std::string& str_name)
{
      std::string parent_name;
      std::string parent_qual_name;

      if (isParentClass(D))
      {    
         if (!getParentName(D, parent_name, parent_qual_name)) return false;

         std::string file_name;
         if (getHasFileNameRule()){
            if (GetDeclSourceFileName(D, file_name))
               std::cout<<"\tSource file name: "<<file_name<<std::endl;
         }

         int fYes = 0, fImplNo = 0;
         bool dontC, noName;
         bool explicit_Yes = false;
         int fFileNo = 0;
         bool file;
         
         //DEBUG std::cout<<"\n\tParent is class";
         std::list<ClassSelectionRule>::iterator it = fClassSelectionRules.begin();
         for(; it != fClassSelectionRules.end(); ++it) {
            bool yes;
            yes = it->isSelected(parent_qual_name, "", file_name, dontC, noName, file, false); // == kYes
            if (yes) {
               ++fYes;
               if (isLinkdefFile()) {
                  if (it->hasAttributeWithName("name")) {
                     std::string name_value;
                     it->getAttributeValue("name", name_value);
                     if (name_value == parent_qual_name) explicit_Yes = true;
                  }
                  if (it->hasAttributeWithName("pattern")) {
                     std::string pattern_value;
                     it->getAttributeValue("pattern", pattern_value);
                     if (pattern_value != "*" && pattern_value != "*::*") explicit_Yes = true;
                  }
               }
            }
            else if (!noName && !dontC) { // == kNo
               if (!isLinkdefFile()) {
                  if (file) ++fFileNo;
                  else {
                     std::cout<<"\tNo returned"<<std::endl;
                     return false; // in genreflex we can't have that situation
                  }
               }
               else {
                  if (it->hasAttributeWithName("pattern")) {
                     std::string pattern_value;
                     it->getAttributeValue("pattern", pattern_value);
                     if (pattern_value == "*" || pattern_value == "*::*") ++fImplNo;
                     else 
                        return false;
                  }
                  else
                     return false;
               }
            }
            else if (dontC ) { // == kDontCare - we check the method and field selection rules for the class
               if (!it->hasMethodSelectionRules() && !it->hasFieldSelectionRules()) {
                  std::cout<<"\tNo fields and methods"<<std::endl;
                  return false; // == kNo
               }
               else {
                  if (kind == "Field" || kind == "CXXMethod" || kind == "CXXConstructor" || kind == "CXXDestructor"){
                     std::list<VariableSelectionRule> members;
                     std::list<VariableSelectionRule>::iterator mem_it;
                     std::list<VariableSelectionRule>::iterator mem_it_end;
        	     std::string prototype;
                     
                     if (kind == "Field") {
                        members = it->getFieldSelectionRules();
                     }
                     else {
      			getFunctionPrototype(D, prototype);
                        prototype = str_name + prototype;
                        std::cout<<"\tIn isMemberSelected (DC)"<<std::endl;
                        members = it->getMethodSelectionRules();
                     }
                     mem_it = members.begin();
                     mem_it_end = members.end();
                     for (; mem_it != mem_it_end; ++mem_it) {
                        if (!mem_it->isSelected(str_name, prototype, file_name, dontC, noName, file, false)) {
                           if (!noName) return false;
                        }                        
                     }
                  }
               }
            }
         }  
                  
         if (isLinkdefFile()) {
            std::cout<<"\n\tfYes = "<<fYes<<", fImplNo = "<<fImplNo<<std::endl;
            if (explicit_Yes) {
               std::cout<<"\tReturning Yes"<<std::endl;
               return true;
            }
            else if (fImplNo > 0) {
               std::cout<<"\tReturning No"<<std::endl;
               return false;
            }
            else {
               std::cout<<"\tReturning Yes"<<std::endl;
               return true;
            }
         }
         else {

            if (fYes > 0) {
               return true;
            }
            else {
               return false;
            }
         }
      }
      else {
         return false;
      }
}

bool SelectionRules::isSelectionXMLFile()
{
   if (fSelectionFileType == kSelectionXMLFile) return true;
   else return false;
}

bool SelectionRules::isLinkdefFile()
{
   if (fSelectionFileType == kLinkdefFile) return true;
   else return false;
}

void SelectionRules::setSelectionFileType(ESelectionFileTypes fileType)
{
   fSelectionFileType = fileType;
   return;
}

bool SelectionRules::areAllSelectionRulesUsed() {
   if (!fClassSelectionRules.empty()) {
      for(std::list<ClassSelectionRule>::iterator it = fClassSelectionRules.begin(); 
          it != fClassSelectionRules.end(); ++it) {
         if (!it->getMatchFound() && !getHasFileNameRule()) {
            std::string name;
            if (it->hasAttributeWithName("name")) it->getAttributeValue("name", name);
            if (it->hasAttributeWithName("pattern")) it->getAttributeValue("pattern", name);

            if (isSelectionXMLFile()){
               std::cout<<"Warning - unused class rule: "<<name<<std::endl;
            }
            else {
               std::cout<<"Error - unused class rule: "<<name<<std::endl;
               return false;
            }
         }
      }
   }
   if (!fVariableSelectionRules.empty()) {
      for(std::list<VariableSelectionRule>::iterator it = fVariableSelectionRules.begin(); 
          it != fVariableSelectionRules.end(); ++it) {
         if (!it->getMatchFound() && !getHasFileNameRule()) {
            std::string name;
            if (it->hasAttributeWithName("name")) it->getAttributeValue("name", name);
            if (it->hasAttributeWithName("pattern")) it->getAttributeValue("pattern", name);

            if (isSelectionXMLFile()){
               std::cout<<"Warning - unused variable rule: "<<name<<std::endl;
            }
            else {
               std::cout<<"Error - unused variable rule: "<<name<<std::endl;
               return false;
            }
         }
      }
   }
   if (!fFunctionSelectionRules.empty()) {
      for(std::list<FunctionSelectionRule>::iterator it = fFunctionSelectionRules.begin(); 
          it != fFunctionSelectionRules.end(); ++it) {
         if (!it->getMatchFound() && !getHasFileNameRule()) {
            std::string name;
            if (it->hasAttributeWithName("name")) it->getAttributeValue("name", name);
            if (it->hasAttributeWithName("pattern")) it->getAttributeValue("pattern", name);
            if (it->hasAttributeWithName("proto_name")) it->getAttributeValue("proto_name", name);
            if (it->hasAttributeWithName("proto_pattern")) it->getAttributeValue("proto_pattern", name);
            if (isSelectionXMLFile()){
               std::cout<<"Warning - unused function rule: "<<name<<std::endl;
            }
            else {
               std::cout<<"Error - unused function rule: "<<name<<std::endl;
               return false;
            }
         }
      }
   }
   if (!fEnumSelectionRules.empty()) {
      for(std::list<EnumSelectionRule>::iterator it = fEnumSelectionRules.begin(); 
          it != fEnumSelectionRules.end(); ++it) {
         if (!it->getMatchFound() && !getHasFileNameRule()) {
            std::string name;
            if (it->hasAttributeWithName("name")) it->getAttributeValue("name", name);
            if (it->hasAttributeWithName("pattern")) it->getAttributeValue("pattern", name);
            if (isSelectionXMLFile()){
               std::cout<<"Warning - unused enum rule: "<<name<<std::endl;
            }
            else {
               std::cout<<"Error - unused enum rule: "<<name<<std::endl;
               return false;
            }
         }
      }
   }
   return true;
}
