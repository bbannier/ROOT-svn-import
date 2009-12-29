#ifndef UTDATASETINFO_H
#define UTDATASETINFO_H

// Author: Christoph Rosemann   Dec. 2009
// TMVA unit tests

#include <vector>
#include "TString.h"
#include "TCut.h"
#include "TMatrixD.h"

#include "DataSetInfo.h"
#include "VariableInfo.h"
#include "UnitTest.h"
#include "ClassInfo.h"
#include "Event.h"

class utDataSetInfo : public UnitTesting::UnitTest
{
 public:
  utDataSetInfo();
  void run();

 private:
  void testConstructor();
  void testMethods();

  TMVA::DataSetInfo* datasetinfo; 

  // the members needed for the creation and comparison of instances
  TString     name;
  TString     expression; 
  TString     title;
  TString     unit;
  Double_t    min;
  Double_t    max;
  Int_t       varcounter;
  char        vartype;
  Bool_t      normalized;
  void*       external;
  TString     norm;
  TString     classname;
  TCut        cut1;
  TCut        cut2;
  TString     splitoption;
  TMatrixD*   matrix;
  TString     histname;
  TString     histtitle;
  TString     weightexpr;

  TMVA::Event*                    event;
  TMVA::ClassInfo*                classinfo;
  TMVA::VariableInfo              varinfo;
  std::vector<TMVA::VariableInfo> vecvarinfo;
};
#endif // UTDATASETINFO_H
