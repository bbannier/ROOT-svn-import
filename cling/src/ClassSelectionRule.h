// Header for the ClassSelection class
#ifndef CLASSSELECTIONRULE_H
#define CLASSSELECTIONRULE_H

#include "BaseSelectionRule.h"
#include "VariableSelectionRule.h"
//#include "FunctionSelectionRule.h"
#include <list>


class ClassSelectionRule: public BaseSelectionRule{
   std::list<VariableSelectionRule> fFieldSelectionRules;
   std::list<FunctionSelectionRule> fMethodSelectionRules;
   bool fIsInheritable;

   bool plus; // for linkdef.h: true if we had '+' at the end of a class name
   bool minus; // for linkdef.h: true if we had '-' or "-!" at the end of a class name
   bool exclamation; // for linkdef.h: true if we had '!' at the end of a class name
   
public:
   ClassSelectionRule():
      BaseSelectionRule(), fIsInheritable(false), plus(false), minus(false), exclamation(false){}
   ClassSelectionRule(bool inherit, ESelect sel, std::string attributeName, std::string attributeValue):
      BaseSelectionRule(sel, attributeName, attributeValue), fIsInheritable(inherit), plus(false), minus(false), exclamation(false){}
   

   void addFieldSelectionRule(VariableSelectionRule field); //adds entry to the filed selections list
   bool hasFieldSelectionRules();
   //const std::list<VariableSelectionRule>& getFieldSelectionRules(); //gets the field selections list
   std::list<VariableSelectionRule>& getFieldSelectionRules(); //gets the field selections list
   
   void addMethodSelectionRule(FunctionSelectionRule method); //adds entry to the method selections list
   bool hasMethodSelectionRules();
   //const std::list<FunctionSelectionRule>& getMethodSelectionRules(); //gets the method selections list
   std::list<FunctionSelectionRule>& getMethodSelectionRules(); //gets the method selections list
   
   bool isInheritable(); //checks if the class selection rule is inheritable
   void setInheritable(bool inherit); //sets the inheritance rule for the class

   bool hasPlus();
   void setPlus(bool pl);

   bool hasMinus();
   void setMinus(bool mn);

   bool hasExclamation();
   void setExclamation(bool excl);   
};

#endif

