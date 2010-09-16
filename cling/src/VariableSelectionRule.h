#ifndef VARIABLESELECTIONRULE_H
#define VARIABLESELECTIONRULE_H

#include "BaseSelectionRule.h"
#include <string>

class VariableSelectionRule:public BaseSelectionRule{
public:
   VariableSelectionRule(){}
   VariableSelectionRule(ESelect sel, std::string attributeName, std::string attributeValue):
      BaseSelectionRule(sel, attributeName, attributeValue){}
};

typedef VariableSelectionRule FunctionSelectionRule; // Function selection rules are the same as Variable selection rules
typedef VariableSelectionRule EnumSelectionRule;     // Enum selection rules are the same as Variable selection rules

#endif
