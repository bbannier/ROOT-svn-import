#include "MethodUnitTestWithROCLimits.h"
#include "TFile.h"
#include "TMVA/MethodBase.h"

/*
#include "TMVA/Tools.h"


#include <cstdlib>
#include <iostream>
#include <map>
#include <string>

#include "TChain.h"
#include "TFile.h"
#include "TTree.h"
#include "TString.h"
#include "TObjString.h"
#include "TSystem.h"
#include "TROOT.h"
#include "TPluginManager.h"

#include "TMVA/Factory.h"
#include "TMVA/Tools.h"
*/

using namespace std;
using namespace UnitTesting;
using namespace TMVA;

MethodUnitTestWithROCLimits::MethodUnitTestWithROCLimits(double upLimit, double lowLimit,
			       const std::string & name,const std::string & filename, std::ostream* sptr) :
  UnitTest(name,filename,sptr), _upROCLimit(upLimit), _lowROCLimit(lowLimit)
{
}


MethodUnitTestWithROCLimits::~MethodUnitTestWithROCLimits()
{
}

bool MethodUnitTestWithROCLimits::ROCIntegralWithinInterval()
{
  return (_ROCValue <= _upROCLimit) && (_ROCValue >= _lowROCLimit);
}

void MethodUnitTestWithROCLimits::run()
{
   //cout << " it's running " << endl;
  TString outfileName( "TMVA.root" );

  TFile* outputFile = TFile::Open( outfileName, "RECREATE" );

  if(!outputFile) // file already exists
    return;    

// absolute silence options:
  string factoryOptions( "!V:Silent:Transformations=I;D;P;G,D:AnalysisType=Classification:!Color:!DrawProgressBar" );
//   std::string factoryOptions( "!V:!Silent:Transformations=I;D;P;G,D:AnalysisType=Classification" );

  Factory* factory = new Factory( "TMVAClassification", outputFile, factoryOptions );
  
  factory->AddVariable( "myvar1 := var1+var2", 'F' );
  factory->AddVariable( "myvar2 := var1-var2", "Expression 2", "", 'F' );
  factory->AddVariable( "var3",                "Variable 3", "units", 'F' );
  factory->AddVariable( "var4",                "Variable 4", "units", 'F' );
  
  //   factory->AddSpectator( "spec1:=var1*2",  "Spectator 1", "units" );
  //   factory->AddSpectator( "spec2:=var1*3",  "Spectator 2", "units" );
  
  
  TFile *input(0);
  TString fname = "../../tmva/test/tmva_example.root"; 
  input = TFile::Open( fname );
  
  if (input == NULL) 
    {
        fname = "../tmva/test/tmva_example.root"; 
        input = TFile::Open( fname );
    }

  if (input == NULL) 
    {
      cerr << "broken/inaccessible input file" << endl;
    }
  
  TTree *signal     = (TTree*)input->Get("TreeS");
  TTree *background = (TTree*)input->Get("TreeB");
  
  Double_t signalWeight     = 1.0;
  Double_t backgroundWeight = 1.0;
  
  factory->AddSignalTree    ( signal,     signalWeight     );
  factory->AddBackgroundTree( background, backgroundWeight );
  
  factory->SetBackgroundWeightExpression("weight");
  
  TCut mycuts = "";
  TCut mycutb = "";
  
  
  factory->PrepareTrainingAndTestTree( mycuts, mycutb,
				       "nTrain_Signal=0:nTrain_Background=0:SplitMode=Random:NormMode=NumEvents:!V" );
 
 
   // _theMethod = factory->BookMethod( Types::kCuts, "Cuts","!H:!V:FitMethod=MC:EffSel:SampleSize=2000:VarProp=FSmart");
  factory->BookMethod( Types::kLD, "LD","!H:!V");
 
  factory->TrainAllMethods();
  factory->TestAllMethods();
  factory->EvaluateAllMethods();
  _theMethod = dynamic_cast<TMVA::MethodBase*> (factory->GetMethod("LD"));
  //_theMethod->TestClassification();
  cout << " the roc integral is " << _theMethod->GetROCIntegral() << endl;  
  outputFile->Close();
  delete factory;
}
