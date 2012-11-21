#include "utSphereData.h"
#include "TFile.h"
#include "TSystem.h"
#include "TMVA/MethodBase.h"

using namespace std;
using namespace UnitTesting;
using namespace TMVA;

utSphereData::utSphereData(const TString& treestring, const TString& preparestring, const Types::EMVA& theMethod, const TString& methodTitle, const TString& theOption,double lowLimit, double upLimit,std::ostream* /* sptr */) :
   UnitTest(string("SphereTutorial_")+(string)methodTitle, __FILE__), _methodType(theMethod) , _treeString(treestring), _prepareString(preparestring), _methodTitle(methodTitle), _methodOption(theOption), _upROCLimit(upLimit), _lowROCLimit(lowLimit)
{
}


utSphereData::~utSphereData()
{
}

bool utSphereData::ROCIntegralWithinInterval()
{
   if (_ROCValue > _upROCLimit) 
      std::cout << "Improvement of method beyond upper limit (counted as \"failure\"): " 
                << _methodTitle
                << ", ROC integral = " << _ROCValue  
                << " low limit="       << _lowROCLimit
                << " high limit="      << _upROCLimit << std::endl;
   return (_ROCValue <= _upROCLimit) && (_ROCValue >= _lowROCLimit);
}

void utSphereData::run()
{
	// FIXME:: create _this_ file or rather somewhere else?
  TString outfileName( "weights/TMVA.root" );
  TFile* outputFile = TFile::Open( outfileName, "RECREATE" );

// FIXME:: if file can't be created do something more?
  if(!outputFile)
    return;    

// FIXME:: make the factory option mutable?
// absolute silence options:
#ifdef VERBOSE
  string factoryOptions( "V:!Silent:Transformations=I;D;P;G;N;U:AnalysisType=Classification:!Color:!DrawProgressBar" );
#else
  string factoryOptions( "!V:Silent:Transformations=I;D;P;G;N;U:AnalysisType=Classification:!Color:!DrawProgressBar" );
#endif
  Factory* factory = new Factory( "TMVASphereData", outputFile, factoryOptions );
  
  factory->AddVariable( "var0",  "Variable 0", 'F' );
  factory->AddVariable( "var1",  "Variable 1", 'F' );
  factory->AddVariable( "var2",  "Variable 2", 'F' );
  
  TFile* input(0);
  TString fname = "testData.root";
  input = TFile::Open(fname);
  if (input == NULL){
     gSystem->Exec("wget http://www.uni-bonn.de/~etoerne/tmva/testData.root" );
     input = TFile::Open("testData.root");
  }
  if (input == NULL) {
     cerr << "broken/inaccessible input file " <<fname<< endl;
  }
  factory->AddSignalTree((TTree*)input->Get("TreeS"));
  factory->AddBackgroundTree((TTree*)input->Get("TreeB"));
  
  TCut mycuts = "";
  TCut mycutb = "";
  if (_prepareString=="") _prepareString = "nTrain_Signal=200:nTrain_Background=200:SplitMode=Random:NormMode=NumEvents:!V" ;
  factory->PrepareTrainingAndTestTree( mycuts, mycutb, _prepareString);
 
  factory->BookMethod(_methodType, _methodTitle, _methodOption);

  factory->TrainAllMethods();
  factory->TestAllMethods();
  factory->EvaluateAllMethods();

  _theMethod = dynamic_cast<TMVA::MethodBase*> (factory->GetMethod(_methodTitle));

  if (_methodType == TMVA::Types::kCuts) {
     // ToDo make class variable _theEffi
     Double_t err=0.;
     Double_t effi = _theMethod->GetEfficiency("Efficiency:0.1", Types::kTesting,err);
#ifdef COUTDEBUG
     std::cout << "Cuts Signal effi at for Background effi of 0.1 = " << effi<<" low limit="<<_lowROCLimit<<" high limit="<<_upROCLimit<<std::endl;
#endif
     test_(effi <= _upROCLimit && effi>=_lowROCLimit);
  }
  else {
     _ROCValue = _theMethod->GetROCIntegral();
#ifdef COUTDEBUG
     std::cout << "ROC integral = "<<_ROCValue <<" low limit="<<_lowROCLimit<<" high limit="<<_upROCLimit<<std::endl;
#endif     
     test_(ROCIntegralWithinInterval());
  }
  outputFile->Close();
  if (outputFile) delete outputFile;
  delete factory;
  input->Close();
  if (input) delete input;
}

