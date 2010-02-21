#ifndef UTVARIABLEINFO_H
#define UTVARIABLEINFO_H

// Author: Christoph Rosemann   Dec. 2009
// TMVA unit tests

#include <vector>

#include "TString.h"

#include "UnitTest.h"

namespace TMVA {
   class VariableInfo;
}


class utVariableInfo : public UnitTesting::UnitTest
{
 public:
  utVariableInfo();
  void run();

 private:
  // the test calls in different blocks
  // the distinctions are arbitrary:
  void _testConstructor1();
  void _testConstructor2();
  void _testConstructor3();

  void _testMethods();
  
  // there are six different constructors:
  TMVA::VariableInfo* _varinfoC1; 
  TMVA::VariableInfo* _varinfoC2; 
  TMVA::VariableInfo* _varinfoC3; 

  // the values needed to create all "VariableInfo" objects in all ways
  TString  expression;
  TString  title;
  TString  unit;
  Int_t    varCounter;
  char     varType;
  Double_t min;
  Double_t max;
  Bool_t   normalized;
  void*    external;
  Float_t  mean;
  Float_t  rms;
};
#endif // UTVARIABLEINFO_H
