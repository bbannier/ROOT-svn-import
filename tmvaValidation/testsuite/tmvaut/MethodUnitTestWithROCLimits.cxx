#include "MethodUnitTestWithROCLimits.h"
#include "TFile.h"
#include "TMVA/MethodBase.h"

using namespace std;
using namespace UnitTesting;
using namespace TMVA;

MethodUnitTestWithROCLimits::MethodUnitTestWithROCLimits(const Types::EMVA& theMethod, const TString& methodTitle, const TString& theOption,
														double lowLimit, double upLimit,
														const std::string & name,const std::string & filename, std::ostream* sptr) :
 UnitTest((string)methodTitle, __FILE__), _methodType(theMethod) , _methodTitle(methodTitle), _methodOption(theOption), _upROCLimit(upLimit), _lowROCLimit(lowLimit)
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
	// FIXME:: create _this_ file or rather somewhere else?
  TString outfileName( "TMVA.root" );
  TFile* outputFile = TFile::Open( outfileName, "RECREATE" );

// FIXME:: if file can't be created do something more?
  if(!outputFile)
    return;    

// FIXME:: make the factory option mutable?
// absolute silence options:
  string factoryOptions( "!V:Silent:Transformations=I;D;P;G,D:AnalysisType=Classification:!Color:!DrawProgressBar" );

  Factory* factory = new Factory( "TMVAUnitTesting", outputFile, factoryOptions );
  
  factory->AddVariable( "myvar1 := var1+var2", 'F' );
  factory->AddVariable( "myvar2 := var1-var2", "Expression 2", "", 'F' );
  factory->AddVariable( "var3",                "Variable 3", "units", 'F' );
  factory->AddVariable( "var4",                "Variable 4", "units", 'F' );
  
  TFile* input(0);
// FIXME:: give the filename of the sample somewhere else?
  TString fname = "../tmva/test/tmva_example.root"; 
  TString fname2 = TString("../")+fname;

  input = TFile::Open( fname );  
  if (input == NULL) input = TFile::Open( fname2 );
  if (input == NULL) 
    {
      cerr << "broken/inaccessible input file" << endl;
    }
  
  TTree *signal     = (TTree*)input->Get("TreeS");
  TTree *background = (TTree*)input->Get("TreeB");
  
  factory->AddSignalTree(signal);  
  factory->AddBackgroundTree(background);
  
  factory->SetBackgroundWeightExpression("weight");
  
  TCut mycuts = "";
  TCut mycutb = "";
  
  // FIXME:: make options string mutable?
  factory->PrepareTrainingAndTestTree( mycuts, mycutb,
				       "nTrain_Signal=1000:nTrain_Background=1000:SplitMode=Random:NormMode=NumEvents:!V" );
 
  factory->BookMethod(_methodType, _methodTitle, _methodOption);

  factory->TrainAllMethods();
  factory->TestAllMethods();
  factory->EvaluateAllMethods();

  _theMethod = dynamic_cast<TMVA::MethodBase*> (factory->GetMethod(_methodTitle));

  if (_methodType == TMVA::Types::kCuts) {
     // ToDo make class variable _theEffi
     Double_t err=0.;
     Double_t effi = _theMethod->GetEfficiency("Efficiency:0.1", Types::kTesting,err);
     std::cout << "Cuts Signal effi at for Background effi of 0.1 = " << effi<<std::endl;
     test_(effi <= _upROCLimit && effi>=_lowROCLimit);
  }
  else {
     _ROCValue = _theMethod->GetROCIntegral();
     std::cout << "ROC integral = "<<_ROCValue <<std::endl;
     
     test_(ROCIntegralWithinInterval());
  }
  outputFile->Close();
  delete factory;
}
