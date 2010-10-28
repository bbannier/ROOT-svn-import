#include "RegressionUnitTestWithDeviation.h"
#include "TFile.h"
#include "TMVA/MethodBase.h"

using namespace std;
using namespace UnitTesting;
using namespace TMVA;

RegressionUnitTestWithDeviation::RegressionUnitTestWithDeviation(const Types::EMVA& theMethod, const TString& methodTitle, const TString& theOption,
                                                                 double lowFullLimit, double upFullLimit,double low90PercentLimit, double up90PercentLimit,
                                                                 const std::string & name,const std::string & filename, std::ostream* sptr) 
   : UnitTest(string("Regression_")+(string)methodTitle, __FILE__), _methodType(theMethod) , _methodTitle(methodTitle), _methodOption(theOption), 
                                                                                                                                  _lowerFullDeviationLimit(lowFullLimit),  _upperFullDeviationLimit(upFullLimit), _lower90PercentDeviationLimit(low90PercentLimit), _upper90PercentDeviationLimit(up90PercentLimit)
{
}


RegressionUnitTestWithDeviation::~RegressionUnitTestWithDeviation()
{
}

bool RegressionUnitTestWithDeviation::DeviationWithinLimits()
{
   return (_the90PercentDeviation<= _upper90PercentDeviationLimit) && (_the90PercentDeviation >= _lower90PercentDeviationLimit) && (_theFullDeviation <= _upperFullDeviationLimit) && (_theFullDeviation>= _lowerFullDeviationLimit);
}

void RegressionUnitTestWithDeviation::run()
{
	// FIXME:: create _this_ file or rather somewhere else?
  TString outfileName( "TMVARegUT.root" );
  TFile* outputFile = TFile::Open( outfileName, "RECREATE" );

// FIXME:: if file can't be created do something more?
  if(!outputFile)
    return;    

// FIXME:: make the factory option mutable?
// absolute silence options:
  string factoryOptions( "!V:Silent:Transformations=I;D;P;G,D:AnalysisType=Regression:!Color:!DrawProgressBar" );

  Factory* factory = new Factory( "TMVARegressionUnitTesting", outputFile, factoryOptions );
  
  factory->AddVariable( "var1", "Variable 1", "units", 'F' );
  factory->AddVariable( "var2", "Variable 2", "units", 'F' );
  
  factory->AddTarget  ( "fvalue" ); 
  
  TFile* input(0);
// FIXME:: give the filename of the sample somewhere else?
  TString fname = "../tmva/test/tmva_reg_example.root"; 
  TString fname2 = TString("../")+fname;

  input = TFile::Open( fname );  
  if (input == NULL) input = TFile::Open( fname2 );
  if (input == NULL) input = TFile::Open( "http://root.cern.ch/files/tmva_reg_example.root" );
  if (input == NULL) 
    {
      cerr << "broken/inaccessible input file" << endl;
    }
  
  TTree *regTree = (TTree*)input->Get("TreeR");
  
  Double_t regWeight  = 1.0;   
  factory->AddRegressionTree( regTree, regWeight );
  factory->SetWeightExpression( "var1", "Regression" );
  TCut mycut = ""; // for example: TCut mycut = "abs(var1)<0.5 && abs(var2-0.5)<1";
     
  factory->PrepareTrainingAndTestTree( mycut, "nTrain_Regression=500:nTest_Regression=500:SplitMode=Random:NormMode=NumEvents:!V" );
                                             
  factory->BookMethod(_methodType, _methodTitle, _methodOption);

  factory->TrainAllMethods();
  factory->TestAllMethods();
  factory->EvaluateAllMethods();

  _theMethod = dynamic_cast<TMVA::MethodBase*> (factory->GetMethod(_methodTitle));

  _theMethod->GetRegressionDeviation(0,TMVA::Types::kTesting, _theFullDeviation,_the90PercentDeviation);
#ifdef COUTDEBUG
  cout << "deviation, dev90= " << _theFullDeviation << ", " <<  _the90PercentDeviation << endl; 
  cout << "Full limits " << _lowerFullDeviationLimit      << " "
       << _upperFullDeviationLimit 
       << ", 90% limits "  << _lower90PercentDeviationLimit << " "
       << _upper90PercentDeviationLimit << endl;
#endif
  test_(DeviationWithinLimits());

    outputFile->Close();
  delete factory;
}
