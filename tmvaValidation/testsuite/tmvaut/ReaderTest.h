#ifndef READERTEST_H
#define READERTEST_H
// author Eckhard von Toerne, U. of Bonn 2011

#include "TFile.h"
#include "TString.h"
#include "TSystem.h"
#include "TMVA/MethodBase.h"
#include "TMVA/Reader.h"
#include <cstdlib>
#include <vector>
#include <string>
#include <iostream>
#include <cassert>

#include "TTree.h"
#include "TString.h"

#include "TMVA/Factory.h"
#include "TMVA/MethodBase.h"
#include "TMVA/Types.h"

#include "UnitTest.h"

namespace UnitTesting
{

// usage
//ReaderTest(weightfile, tmvafile, treename, varlist)

class ReaderTest : public UnitTest {
public:
   ReaderTest(TString methodtitle, 
                    TString weightfile, 
                    TString tmvafile, 
                    TString treename ,
              const std::vector<TString>* varlist, 
              bool isRegression=false);

   ReaderTest(TString methodtitle, 
              TString weightdir,
              bool isRegression=false);
 
   virtual ~ReaderTest();
   void OpenFiles();
   virtual void run();

private:
   bool fIsRegression;
   TString fWeightFile, fTreeFilename,fTreeName,fMethodTitle;
   const std::vector<TString>* fVariableNames;
   const std::vector<TString>* fBranchNames;
   const std::vector<TString>* fSpecNames;
   TFile* fTestFile;
   TTree* fTestTree;
   UInt_t fNVar;
   UInt_t fNSpecs;
   static const int fMaxEvt=50;
};

} // namespace UnitTesting

#endif
