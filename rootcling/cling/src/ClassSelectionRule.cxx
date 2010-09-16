#include "ClassSelectionRule.h"

void ClassSelectionRule::addFieldSelectionRule(VariableSelectionRule field)
{
  fFieldSelectionRules.push_back(field);
}

bool ClassSelectionRule::hasFieldSelectionRules()
{
  return !fFieldSelectionRules.empty();
}

//const std::list<VariableSelectionRule>& ClassSelectionRule::getFieldSelectionRules()
std::list<VariableSelectionRule>& ClassSelectionRule::getFieldSelectionRules()
{
  return fFieldSelectionRules;
}

void ClassSelectionRule::addMethodSelectionRule(FunctionSelectionRule method)
{
  fMethodSelectionRules.push_back(method);
}

bool ClassSelectionRule::hasMethodSelectionRules()
{
  return !fMethodSelectionRules.empty();
}

//const std::list<FunctionSelectionRule>& ClassSelectionRule::getMethodSelectionRules()
std::list<FunctionSelectionRule>& ClassSelectionRule::getMethodSelectionRules()
{
  return fMethodSelectionRules;
}

bool ClassSelectionRule::isInheritable()
{
  return fIsInheritable;
}

void ClassSelectionRule::setInheritable(bool inherit)
{
  fIsInheritable = inherit;
}

bool ClassSelectionRule::hasPlus()
{
   return plus;
}

void ClassSelectionRule::setPlus(bool pl)
{
   plus = pl;
}

bool ClassSelectionRule::hasMinus()
{
   return minus;
}

void ClassSelectionRule::setMinus(bool mn)
{
   minus = mn;
}

bool ClassSelectionRule::hasExclamation()
{
   return exclamation;
}

void ClassSelectionRule::setExclamation(bool excl)
{
   exclamation = excl;
}


