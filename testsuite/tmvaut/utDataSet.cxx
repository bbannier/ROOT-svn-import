
#include "utDataSet.h"

#include "TMVA/VariableInfo.h"
#include "TMVA/Types.h"
#include "TMVA/Event.h"
#include "TMVA/DataSet.h"
#include "TMVA/DataSetInfo.h"

#include "TString.h"

using namespace std;
using namespace TMVA;

utDataSet::utDataSet() :
   UnitTest("DataSet", __FILE__)
{
   //   event;
   //   vecevent;
   //  results;

   TString xname       = "name";
   TString expression1 = "expression1";
   TString expression2 = "expression2";
   TString expression3 = "expression3";
   TString title      = "title";
   TString unit       = "unit";
   char    vartype    = 'D';
   Float_t min        = 2.781828;
   Float_t max        = 3.1416;
   Bool_t  normalized = kFALSE;
   TString classname  = "classname";
   void* external = &max;
   UInt_t   _testClassVal    = 2;

   // the test values for initialisation
   vector<Float_t> _testValueVec, _testTargetVec, _testSpectatorVec;
   _testValueVec.push_back(1.);
   _testValueVec.push_back(2.);
   _testValueVec.push_back(3.);
   _testTargetVec.push_back(11.);
   _testTargetVec.push_back(12.);
   _testTargetVec.push_back(13.);
   _testSpectatorVec.push_back(25.);
   Float_t  _testWeight      = 3.1415;
   Float_t _testBoostWeight = 0.1234;
   event0 = new Event( _testValueVec, _testTargetVec, _testSpectatorVec, _testClassVal, _testWeight, _testBoostWeight);
   event1 = new Event( _testValueVec, _testTargetVec, _testSpectatorVec, _testClassVal, _testWeight, _testBoostWeight);
   event2 = new Event( _testValueVec, _testTargetVec, _testSpectatorVec, _testClassVal, _testWeight, _testBoostWeight);
   event3 = new Event( _testValueVec, _testTargetVec, _testSpectatorVec, _testClassVal, _testWeight, _testBoostWeight);
   event4 = new Event( _testValueVec, _testTargetVec, _testSpectatorVec, _testClassVal, _testWeight, _testBoostWeight);

   UInt_t  varcounter = 0;

   VariableInfo* varinfo1     = new VariableInfo( expression1, title,  unit, varcounter++, vartype, external, min, max, normalized);
   VariableInfo* varinfo2     = new VariableInfo( expression2, title,  unit, varcounter++, vartype, external, min, max, normalized);
   VariableInfo* varinfo3     = new VariableInfo( expression3, title,  unit, varcounter++, vartype, external, min, max, normalized);
   datasetinfo               = new DataSetInfo(xname);
   datasetinfo->AddVariable(*varinfo1);
   datasetinfo->AddVariable(*varinfo2);
   datasetinfo->AddVariable(*varinfo3);
   datasetinfo->AddTarget(*varinfo1);
   datasetinfo->AddTarget(*varinfo2);
   datasetinfo->AddTarget(*varinfo3);
   datasetinfo->AddSpectator(*varinfo1);

   dataset = new DataSet(*datasetinfo);
}



void utDataSet::run()
{
  testMethods();
}



void utDataSet::testMethods()
{
   test_(dataset->GetNEvents() == 0);
   dataset->AddEvent(event0, Types::kTraining);
   dataset->AddEvent(event1, Types::kTraining);
   dataset->AddEvent(event2, Types::kTesting);
   dataset->AddEvent(event3, Types::kMaxTreeType);
   dataset->AddEvent(event4, Types::kValidation);

   test_(dataset->GetNEvents(Types::kTraining) == 2);
   test_(dataset->GetNEvents(Types::kTesting) == 1);
   //test_(dataset->GetNEvents(Types::kMaxTreeType) == 1); //ToDo check this isuue
   test_(dataset->GetNEvents(Types::kValidation) == 1);
   //test_(dataset->GetNEvents(Types::kTrainingOriginal) == 0);//ToDo check this issue

   test_(dataset->GetNTrainingEvents() == 2);
   test_(dataset->GetNTestEvents() == 1);

   test_(dataset->GetNVariables()  == 3);
   test_(dataset->GetNTargets()    == 3);
   test_(dataset->GetNSpectators() == 1);

   // this is only a temporary solution for the testing
   // FIXME:: Extend to a real test!

   test_(dataset->GetEvent());
   test_(dataset->GetEvent(0));
   test_(dataset->GetTrainingEvent(0));
   test_(dataset->GetTestEvent(0));
   test_(dataset->GetEvent(1, Types::kTraining));

   // calls to these function will cause the program to crash
   //  test_(dataset->GetNEvtSigTest() == 1);
   //  test_(dataset->GetNEvtBkgdTest() == 1);
   //  test_(dataset->GetNEvtSigTrain() == 1);
   //   test_(dataset->GetNEvtBkgdTrain() == 1);

   test_(dataset->HasNegativeEventWeights() == kFALSE);

   /* function still to develop tests for:
      void      SetCurrentEvent( Long64_t ievt         ) const { fCurrentEventIdx = ievt; }
      void      SetCurrentType ( Types::ETreeType type ) const { fCurrentTreeIdx = TreeIndex(type); }

      void                       SetEventCollection( std::vector<Event*>*, Types::ETreeType );
      const std::vector<Event*>& GetEventCollection( Types::ETreeType type = Types::kMaxTreeType ) const;
      const TTree*               GetEventCollectionAsTree();

      Results*  GetResults   ( const TString &,Types::ETreeType type, Types::EAnalysisType analysistype );
      void      DivideTrainingSet( UInt_t blockNum );
      void      MoveTrainingBlock( Int_t blockInd,Types::ETreeType dest, Bool_t applyChanges = kTRUE );
      void      IncrementNClassEvents( Int_t type, UInt_t classNumber );
      Long64_t  GetNClassEvents      ( Int_t type, UInt_t classNumber );
      void      ClearNClassEvents    ( Int_t type );
      TTree*    GetTree( Types::ETreeType type );
      void      InitSampling( Float_t fraction, Float_t weight, UInt_t seed = 0 );
      void      EventResult( Bool_t successful, Long64_t evtNumber = -1 );
      void      CreateSampling() const;
      UInt_t    TreeIndex(Types::ETreeType type) const;
   */
}


