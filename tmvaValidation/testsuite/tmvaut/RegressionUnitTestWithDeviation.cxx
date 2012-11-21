#include "RegressionUnitTestWithDeviation.h"
#include "TFile.h"
#include "TSystem.h"
#include "TMVA/MethodBase.h"
#include "TMVA/Reader.h"
#include <cstdlib>

using namespace std;
using namespace UnitTesting;
using namespace TMVA;

RegressionUnitTestWithDeviation::RegressionUnitTestWithDeviation(const Types::EMVA& theMethod, const TString& methodTitle, const TString& theOption,
                                                                 double lowFullLimit, double upFullLimit,double low90PercentLimit, double up90PercentLimit,
                                                                 const std::string & /* xname */ ,const std::string & /* filename */ , std::ostream* /* sptr */) 
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
   TString outfileName( "weights/TMVARegUT.root" );
   TFile* outputFile = TFile::Open( outfileName, "RECREATE" );

   // FIXME:: if file can't be created do something more?
   if(!outputFile)
      return;

   // FIXME:: make the factory option mutable?
   // absolute silence options:
#ifdef VERBOSE
   string factoryOptions( "V:!Silent:Transformations=I;D;P;G;N;U:AnalysisType=Regression:!Color:!DrawProgressBar" );
#else
   string factoryOptions( "!V:Silent:Transformations=I;D;P;G;N;U:AnalysisType=Regression:!Color:!DrawProgressBar" );
#endif
   Factory* factory = new Factory( "TMVARegressionUnitTesting", outputFile, factoryOptions );

   factory->AddVariable( "var1", "Variable 1", "units", 'F' ); // fix me
   factory->AddVariable( "var2", "Variable 2", "units", 'F' ); // fix me
   TString _targetname="fvalue";
   factory->AddTarget  ( _targetname.Data() ); // fix me _targetname.Data()

   TFile* input(0);
   FileStat_t stat;

   // FIXME:: give the filename of the sample somewhere else?
   TString fname = "../tmva/test/tmva_reg_example.root";
   if(!gSystem->GetPathInfo(fname,stat)) {
      input = TFile::Open( fname );
   } else if(!gSystem->GetPathInfo("../"+fname,stat)) {
      input = TFile::Open( "../"+fname );
   } else {
      input = TFile::Open( "http://root.cern.ch/files/tmva_reg_example.root" );
   }
   if (input == NULL) {
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
   if (DeviationWithinLimits()){
#ifdef COUTDEBUG
   cout << "deviation, dev90= " << _theFullDeviation << ", " <<  _the90PercentDeviation << endl;
   cout << "Full limits " << _lowerFullDeviationLimit      << " "
        << _upperFullDeviationLimit
        << ", 90% limits "  << _lower90PercentDeviationLimit << " "
        << _upper90PercentDeviationLimit << endl;
#endif
   }
   else { 
      if (_the90PercentDeviation < _lower90PercentDeviationLimit || 
          _theFullDeviation      < _lowerFullDeviationLimit ){
         cout << "Improvement of method beyond limit (counted as \"failure\"): "<<_methodTitle<<", deviation, dev90= " << _theFullDeviation << ", " <<  _the90PercentDeviation << endl;
         cout << "Full limits " << _lowerFullDeviationLimit      << " "
              << _upperFullDeviationLimit
              << ", 90% limits "  << _lower90PercentDeviationLimit << " "
              << _upper90PercentDeviationLimit << endl;
      }
      else {
         cout << "Failure "<<_methodTitle<<", deviation, dev90= " << _theFullDeviation << ", " <<  _the90PercentDeviation << endl;
         cout << "Full limits " << _lowerFullDeviationLimit      << " "
              << _upperFullDeviationLimit
              << ", 90% limits "  << _lower90PercentDeviationLimit << " "
              << _upper90PercentDeviationLimit << endl;
      }
   }
   test_(DeviationWithinLimits());
   
   outputFile->Close();
   delete factory;
   input->Close();
   if (input) delete input;

   // reader tests

   // setup test tree access
   TFile* testFile = new TFile("weights/TMVARegUT.root"); // fix me hardcoded file name
   TTree* testTree = (TTree*)(testFile->Get("TestTree"));
   const int nTest=3; // 3 reader usages
   float testTarget,readerVal=0.;

   vector<TString>* _VariableNames = new std::vector<TString>(0); // fix me, move to constructor
   _VariableNames->push_back("var1");
   _VariableNames->push_back("var2");

   vector<float>  testvar(_VariableNames->size());
   vector<float>  dummy(_VariableNames->size());
   vector<float>  dummy2(_VariableNames->size());
   vector<float>  testvarFloat(_VariableNames->size());
   vector<double> testvarDouble(_VariableNames->size());
   for (UInt_t i=0;i<_VariableNames->size();i++)
      testTree->SetBranchAddress(_VariableNames->at(i),&testvar[i]);
   testTree->SetBranchAddress(_methodTitle.Data(),&testTarget);

   TString readerName = _methodTitle + TString(" method");
   TString dir    = "weights/TMVARegressionUnitTesting_";
   TString weightfile=dir+_methodTitle+".weights.xml";
   double diff, maxdiff = 0., sumdiff=0., previousVal=0.;
   int stuckCount=0, nevt= TMath::Min((int) testTree->GetEntries(),50);

   std::vector< TMVA::Reader* > reader(nTest);
   for (int iTest=0;iTest<nTest;iTest++){
      //std::cout << "iTest="<<iTest<<std::endl;
      reader[iTest] = new TMVA::Reader( "!Color:Silent" );
      for (UInt_t i=0;i<_VariableNames->size();i++)
         reader[iTest]->AddVariable( _VariableNames->at(i),&testvar[i]);

      reader[iTest] ->BookMVA( readerName, weightfile) ;

      // run the reader application and compare to test tree
      for (Long64_t ievt=0;ievt<nevt;ievt++) {
         testTree->GetEntry(ievt);
         for (UInt_t i=0;i<_VariableNames->size();i++){
            testvarDouble[i]= testvar[i];
            testvarFloat[i]= testvar[i];
         }

         if (iTest==0){ readerVal=(reader[iTest]->EvaluateRegression( readerName))[0];}
         else if (iTest==1){ readerVal=(reader[iTest]->EvaluateRegression( readerName)).at(0);}
         else if (iTest==2){ readerVal=reader[iTest]->EvaluateRegression( 0, readerName);}
         else {
            std::cout << "ERROR, undefined iTest value "<<iTest<<endl;
            exit(1);
         }

         diff = TMath::Abs(readerVal-testTarget);
         maxdiff = diff > maxdiff ? diff : maxdiff;
         sumdiff += diff;
         if (ievt>0 && iTest ==0 && TMath::Abs(readerVal-previousVal)<1.e-6) stuckCount++;
         //if (ievt<3) std::cout << "i="<<iTest<<", readerVal="<<readerVal<<" testTarget"<<testTarget<<" diff="<<diff<<std::endl;

         if (iTest ==0 ) previousVal=readerVal;
      }

   }

   sumdiff=sumdiff/nevt;

   test_(maxdiff <1.e-4);
   test_(sumdiff <1.e-5);
   test_(stuckCount<nevt/10);

   testFile->Close();
   if (testFile) delete testFile;

   for (int i=0;i<nTest;i++) delete reader[i];

#ifdef COUTDEBUG
   cout << "end of reader test maxdiff="<<maxdiff<<", sumdiff="<<sumdiff<<" stuckcount="<<stuckCount<<endl;
#endif
}
