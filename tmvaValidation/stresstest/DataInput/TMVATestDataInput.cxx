// @(#)root/tmva $Id$
/**********************************************************************************
 * Project   : TMVA - a Root-integrated toolkit for multivariate data analysis    *
 * Package   : TMVA                                                               *
 * Exectuable: TMVATestDataInput                                                 *
 *                                                                                *
 * This executable tests several methods of providing input data to the factory
 * author E. von Toerne, U. of Bonn                                               *
 **********************************************************************************/

#include <cstdlib>
#include <iostream> 
#include <map>
#include <vector>
#include <string>
#include <cassert>

#include "TChain.h"
#include "TFile.h"
#include "TTree.h"
#include "TString.h"
#include "TH1D.h"
#include "TObjString.h"
#include "TSystem.h"
#include "TROOT.h"
#include "TPluginManager.h"

#include "TMVA/Factory.h"
#include "TMVA/Tools.h"
#include "TMVA/Types.h"

#include "DataInputTest.h"

//_______________________________________________________________________
void PrepareVector( TTree* tree, std::vector<Float_t>& event )
{
   event.resize(7);
   tree->SetBranchAddress("var1",&event[0]);
   tree->SetBranchAddress("var2",&event[1]);
   tree->SetBranchAddress("var3",&event[2]);
   tree->SetBranchAddress("var4",&event[3]);
   tree->SetBranchAddress("issig",&event[4]);
   tree->SetBranchAddress("istest",&event[5]);
   tree->SetBranchAddress("isfake",&event[6]);
}

//_______________________________________________________________________
void CopyToDouble(std::vector<Float_t>& fevent, std::vector<Double_t>& devent)
{ // ToDo make this function obsolete at some point
   devent.resize(fevent.size());
   for (int i=0;i<fevent.size();i++) devent[i]=fevent[i];
}

//_______________________________________________________________________
void SetInputData(int index, TMVA::Factory* factory, DataInputTest* thetest) 
{
   std::cout <<" -------------------------------" <<std::endl;
   std::cout <<" this is SetInputData index="<<index<<std::endl;
   std::cout <<" -------------------------------" <<std::endl;
   // load the signal and background event samples from ROOT trees
   TFile *input(0);
   TString fname = "./TMVAInputData.root";
   input = TFile::Open( fname );
   assert(input);
   
   TTree* TreeS1Train   =(TTree*)input->Get( "TreeS1Train");
   TTree* TreeS2Train   =(TTree*)input->Get( "TreeS2Train");
   TTree* TreeSSumTrain =(TTree*)input->Get( "TreeSSumTrain");
   TTree* TreeS1Test    =(TTree*)input->Get( "TreeS1Test");
   TTree* TreeS2Test    =(TTree*)input->Get( "TreeS2Test");
   TTree* TreeSSumTest  =(TTree*)input->Get( "TreeSSumTest");
   TTree* TreeSFake     =(TTree*)input->Get( "TreeSFake");

   TTree* TreeB1Train   =(TTree*)input->Get( "TreeB1Train");
   TTree* TreeB2Train   =(TTree*)input->Get( "TreeB2Train");
   TTree* TreeBSumTrain =(TTree*)input->Get( "TreeBSumTrain");
   TTree* TreeB1Test    =(TTree*)input->Get( "TreeB1Test");
   TTree* TreeB2Test    =(TTree*)input->Get( "TreeB2Test");
   TTree* TreeBSumTest  =(TTree*)input->Get( "TreeBSumTest");
   TTree* TreeBFake     =(TTree*)input->Get( "TreeBFake");

   TTree* TreeSBSumTest =(TTree*)input->Get( "TreeSBSumTest");
   TTree* TreeSBSumTrain=(TTree*)input->Get( "TreeSBSumTrain");
   TTree* TreeSBLarge   =(TTree*)input->Get( "TreeSBLarge");

   TCut dummycut = ""; // for example: TCut mycuts = "abs(var1)<0.5 && abs(var2-0.5)<1";
   TCut mycuts = "";
   TCut mycutb = "";
   std::cout << "Expected behavior: "; 
   int ntrainSig, ntrainBg, ntestSig, ntestBg;
   if (index==0){ 
      factory->AddSignalTree    ( TreeSSumTrain, 1.0, TMVA::Types::kTraining );
      factory->AddBackgroundTree( TreeBSumTrain, 1.0, TMVA::Types::kTraining );
      factory->AddSignalTree    ( TreeSSumTest,  1.0, TMVA::Types::kTesting  );
      factory->AddBackgroundTree( TreeBSumTest, 1.0 , TMVA::Types::kTesting  );
      factory->SetBackgroundWeightExpression("weight");
      factory->PrepareTrainingAndTestTree( dummycut, dummycut,
                                           "nTrain_Signal=0:nTrain_Background=0:SplitMode=Random:NormMode=NumEvents:!V" );      
      std::cout << "strict separation of testing and training, 8000 train 8000 test total, using Nevts=0"<<std::endl;
      // RegisterAssertion(nTrain_Signal,nTrain_Background,nTest_Signal,nTest_Background)
      thetest->RegisterAssertion(4000,4000,4000,4000);
   }
   else if (index==1){ 
      factory->AddBackgroundTree( TreeBSumTest, 1.0 , TMVA::Types::kTesting  );
      factory->AddBackgroundTree( TreeBSumTrain, 1.0, TMVA::Types::kTraining );
      factory->AddSignalTree    ( TreeSSumTest,  1.0, TMVA::Types::kTesting  );
      factory->AddSignalTree    ( TreeSSumTrain, 1.0, TMVA::Types::kTraining );
      factory->SetBackgroundWeightExpression("weight");
      factory->PrepareTrainingAndTestTree( dummycut, dummycut,
                                           "nTrain_Signal=0:nTrain_Background=0:SplitMode=Random:NormMode=NumEvents:!V" );      
      std::cout << "strict separation of testing and training, 8000 train 8000 test total, different order"<<std::endl;
      thetest->RegisterAssertion(4000,4000,4000,4000);
   }
   else if (index==2){ 
      factory->AddSignalTree    ( TreeSSumTrain, 1.0, TMVA::Types::kTraining );
      factory->AddBackgroundTree( TreeBSumTrain, 1.0, TMVA::Types::kTraining );
      factory->AddSignalTree    ( TreeSSumTest,  1.0, TMVA::Types::kTesting  );
      factory->AddBackgroundTree( TreeBSumTest, 1.0 , TMVA::Types::kTesting  );
      factory->SetBackgroundWeightExpression("weight");
      factory->PrepareTrainingAndTestTree( dummycut, dummycut,
                                           "nTrain_Signal=0:nTrain_Background=0:nTest_Signal=4000:nTest_Background=4000:SplitMode=Random:NormMode=NumEvents:!V" );      
      std::cout << "strict separation of testing and training, 8000 train 8000 test total"<<std::endl;
      thetest->RegisterAssertion(4000,4000,4000,4000);
   }
   else if (index==3){ 
      factory->AddSignalTree    ( TreeSSumTrain, 1.0, TMVA::Types::kTraining );
      factory->AddBackgroundTree( TreeBSumTrain, 1.0, TMVA::Types::kTraining );
      factory->AddSignalTree    ( TreeSSumTest,  1.0, TMVA::Types::kTesting  );
      factory->AddBackgroundTree( TreeBSumTest, 1.0 , TMVA::Types::kTesting  );
      factory->SetBackgroundWeightExpression("weight");
      factory->PrepareTrainingAndTestTree( dummycut, dummycut,
                                           "nTrain_Signal=4000:nTest_Background=4000:SplitMode=Random:NormMode=NumEvents:!V" );      
      std::cout << "strict separation of testing and training, 8000 train 8000 test total"<<std::endl;
      thetest->RegisterAssertion(4000,4000,4000,4000);
   }
   else if (index==4){ 
      factory->AddSignalTree    ( TreeSSumTrain, 1.0, TMVA::Types::kTraining );
      factory->AddBackgroundTree( TreeBSumTrain, 1.0, TMVA::Types::kTraining );
      factory->AddSignalTree    ( TreeSSumTest,  1.0, TMVA::Types::kTesting  );
      factory->AddBackgroundTree( TreeBSumTest, 1.0 , TMVA::Types::kTesting  );
      factory->SetBackgroundWeightExpression("weight");
      factory->PrepareTrainingAndTestTree( dummycut, dummycut,
                                           "nTrain_Signal=4000:nTrain_Background=4000:nTest_Signal=4000:nTest_Background=4000:SplitMode=Random:NormMode=NumEvents:!V" );      
      std::cout << "strict separation of testing and training, 8000 train 8000 test total"<<std::endl;
      thetest->RegisterAssertion(4000,4000,4000,4000);
   }
   else if (index==5){ 
      factory->AddSignalTree    ( TreeSSumTrain, 1.0, TMVA::Types::kTraining );
      factory->AddBackgroundTree( TreeBSumTrain, 1.0, TMVA::Types::kTraining );
      factory->AddSignalTree    ( TreeSSumTest,  1.0, TMVA::Types::kTesting  );
      factory->AddBackgroundTree( TreeBSumTest, 1.0 , TMVA::Types::kTesting  );
      factory->SetBackgroundWeightExpression("weight");
      factory->PrepareTrainingAndTestTree( dummycut,
                                           "nTrain_Signal=4000:nTrain_Background=4000:nTest_Signal=4000:nTest_Background=4000:SplitMode=Random:NormMode=NumEvents:!V" );      
      std::cout << "strict separation of testing and training, 8000 train 8000 test total, using PrepareTraining.. with one cut"<<std::endl;
      thetest->RegisterAssertion(4000,4000,4000,4000);
   }
   else if (index==6){ 
      factory->AddSignalTree    ( TreeS1Train, 1.0, TMVA::Types::kTraining );
      factory->AddSignalTree    ( TreeS2Train, 1.0, TMVA::Types::kTraining );
      factory->AddBackgroundTree( TreeBSumTrain, 1.0, TMVA::Types::kTraining );
      factory->AddSignalTree    ( TreeSSumTest,  1.0, TMVA::Types::kTesting  );
      factory->AddBackgroundTree( TreeBSumTest, 1.0 , TMVA::Types::kTesting  );
      factory->SetBackgroundWeightExpression("weight");
      factory->PrepareTrainingAndTestTree( dummycut,
                                           "nTrain_Signal=4000:nTrain_Background=4000:nTest_Signal=4000:nTest_Background=4000:SplitMode=Random:NormMode=NumEvents:!V" );      
      std::cout << "strict separation of testing and training, 8000 train 8000 test total, using PrepareTraining.. with one cut"<<std::endl;
      thetest->RegisterAssertion(4000,4000,4000,4000);
   }
   else if (index==7){ 
      factory->AddSignalTree    ( TreeSSumTrain, 1.0, TMVA::Types::kTraining );
      factory->AddBackgroundTree( TreeBSumTrain, 1.0, TMVA::Types::kTraining );
      factory->AddSignalTree    ( TreeSSumTest,  1.0, TMVA::Types::kTesting  );
      factory->AddBackgroundTree( TreeBSumTest, 1.0 , TMVA::Types::kTesting  );
      factory->SetBackgroundWeightExpression("weight");
      factory->PrepareTrainingAndTestTree( dummycut, dummycut,
                                           "nTrain_Signal=0:nTrain_Background=0:SplitMode=Alternate:NormMode=NumEvents:!V" );      
      std::cout << "strict separation of testing and training, 8000 train 8000 test total, using Nevts=0, splitmode=alternate"<<std::endl;
      thetest->RegisterAssertion(4000,4000,4000,4000);
   }
   else if (index==8){ 
      factory->AddSignalTree    ( TreeSSumTrain, 1.0, TMVA::Types::kTraining );
      factory->AddBackgroundTree( TreeBSumTrain, 1.0, TMVA::Types::kTraining );
      factory->AddSignalTree    ( TreeSSumTest,  1.0, TMVA::Types::kTesting  );
      factory->AddBackgroundTree( TreeBSumTest, 1.0 , TMVA::Types::kTesting  );
      factory->SetBackgroundWeightExpression("weight");
      factory->PrepareTrainingAndTestTree( dummycut, dummycut,
                                           "nTrain_Signal=0:nTrain_Background=0:SplitMode=Block:NormMode=NumEvents:!V" );      
      std::cout << "strict separation of testing and training, 8000 train 8000 test total, using Nevts=0, splitmode=block"<<std::endl;
      thetest->RegisterAssertion(4000,4000,4000,4000);
   }
   else if (index==10) {
      std::cout << "strict separation, 3000 train, 2000 test total"<<std::endl;
      factory->AddSignalTree    ( TreeSSumTrain, 1.0, TMVA::Types::kTraining );
      factory->AddBackgroundTree( TreeBSumTrain, 1.0, TMVA::Types::kTraining );
      factory->AddSignalTree    ( TreeSSumTest,  1.0, TMVA::Types::kTesting  );
      factory->AddBackgroundTree( TreeBSumTest,  1.0, TMVA::Types::kTesting  );
      factory->SetBackgroundWeightExpression("weight");
      factory->PrepareTrainingAndTestTree( dummycut, dummycut,
                                           "nTrain_Signal=1500:nTrain_Background=1500:nTest_Signal=1000:nTest_Background=1000:SplitMode=Random:NormMode=NumEvents:!V" );  
      thetest->RegisterAssertion(1500,1500,1000,1000);
   }
   else if (index==11) {
      std::cout << "strict separation, 3000 train, 2000 test total, signal/background given in two trees"<<std::endl;
      factory->AddSignalTree    ( TreeS1Train,  1.0, TMVA::Types::kTraining );
      factory->AddSignalTree    ( TreeS2Train,  1.0, TMVA::Types::kTraining );
      factory->AddBackgroundTree( TreeB1Train,  1.0, TMVA::Types::kTraining );
      factory->AddBackgroundTree( TreeB2Train,  1.0, TMVA::Types::kTraining );
      factory->AddSignalTree    ( TreeSSumTest, 1.0, TMVA::Types::kTesting  );
      factory->AddBackgroundTree( TreeBSumTest, 1.0, TMVA::Types::kTesting  );
      factory->SetBackgroundWeightExpression("weight");
      factory->PrepareTrainingAndTestTree( dummycut, dummycut,
                                           "nTrain_Signal=1500:nTrain_Background=1500:nTest_Signal=1000:nTest_Background=1000:SplitMode=Random:NormMode=NumEvents:!V" );  
      thetest->RegisterAssertion(1500,1500,1000,1000);
   }
   else if (index==12) {
      std::cout << "strict separation, 3000 train, 2000 test total, weight expresion booked before trees are"<<std::endl;
      factory->SetBackgroundWeightExpression("weight");
      factory->AddSignalTree    ( TreeS1Train,  1.0, TMVA::Types::kTraining );
      factory->AddSignalTree    ( TreeS2Train,  1.0, TMVA::Types::kTraining );
      factory->AddBackgroundTree( TreeB1Train,  1.0, TMVA::Types::kTraining );
      factory->AddBackgroundTree( TreeB2Train,  1.0, TMVA::Types::kTraining );
      factory->AddSignalTree    ( TreeSSumTest, 1.0, TMVA::Types::kTesting  );
      factory->AddBackgroundTree( TreeBSumTest, 1.0, TMVA::Types::kTesting  );
      factory->PrepareTrainingAndTestTree( dummycut, dummycut,
                                           "nTrain_Signal=1500:nTrain_Background=1500:nTest_Signal=1000:nTest_Background=1000:SplitMode=Random:NormMode=NumEvents:!V" );  
      thetest->RegisterAssertion(1500,1500,1000,1000);
   }
   else if (index==13) {
      std::cout << "strict separation, 5000 train, 2000 test total, nTrain_Background=0"<<std::endl;
      factory->AddSignalTree    ( TreeSSumTrain, 1.0, TMVA::Types::kTraining );
      factory->AddBackgroundTree( TreeBSumTrain, 1.0, TMVA::Types::kTraining );
      factory->AddSignalTree    ( TreeSSumTest,  1.0, TMVA::Types::kTesting  );
      factory->AddBackgroundTree( TreeBSumTest,  1.0, TMVA::Types::kTesting  );
      factory->SetBackgroundWeightExpression("weight");
      factory->PrepareTrainingAndTestTree( dummycut, dummycut,
                                           "nTrain_Signal=1000:nTrain_Background=0:nTest_Signal=1000:nTest_Background=1000:SplitMode=Random:NormMode=NumEvents:!V" );  
      thetest->RegisterAssertion(1000,4000,1000,1000);
   }
   else if (index==14){ 
      std::cout << "using one tree for S and B, test and train separate, 4000 train 4000 test total"<<std::endl;
      factory->AddTree    ( TreeSBSumTrain,"Signal", 1.0, "issig>0.5",  TMVA::Types::kTraining  );
      factory->AddTree    ( TreeSBSumTrain,"Background", 1.0, "issig<0.5", TMVA::Types::kTraining  );
      factory->AddTree    ( TreeSBSumTest,"Signal", 1.0,  "issig>0.5", TMVA::Types::kTesting  );
      factory->AddTree    ( TreeSBSumTest,"Background",1.0, "issig<0.5", TMVA::Types::kTesting  );
      factory->SetBackgroundWeightExpression("weight");
      factory->PrepareTrainingAndTestTree( dummycut, dummycut,
                                           "nTrain_Signal=0:nTrain_Background=0:nTest_Signal=0:nTest_Background=0:SplitMode=Random:NormMode=NumEvents:!V" );      
      thetest->RegisterAssertion(4000,4000,4000,4000);
   }
   else if (index==15){ 
      std::cout << "using one tree for S and B, test and train separate, 4000 train 4000 test total"<<std::endl;
      factory->AddTree    ( TreeSBSumTrain,"Signal", 1.0, "issig>0.5",  TMVA::Types::kTraining  );
      factory->AddTree    ( TreeSBSumTrain,"Background", 1.0, "issig<0.5", TMVA::Types::kTraining  );
      factory->AddTree    ( TreeSBSumTest,"Signal", 1.0,  "issig>0.5", TMVA::Types::kTesting  );
      factory->AddTree    ( TreeSBSumTest,"Background",1.0, "issig<0.5", TMVA::Types::kTesting  );
      factory->SetBackgroundWeightExpression("weight");
      factory->PrepareTrainingAndTestTree( dummycut, dummycut,
                                           "nTrain_Signal=1001:nTrain_Background=1002:nTest_Signal=1003:nTest_Background=1004:SplitMode=Random:NormMode=NumEvents:!V" );      
      thetest->RegisterAssertion(1001,1002,1003,1004);
   }
   else if (index==16){
      std::cout << "Test with splitmode block, using four separate trees, strict separation of testing and training, 8000 train 8000 test total, using Nevts=0"<<std::endl;
      factory->AddSignalTree    ( TreeSSumTrain, 1.0, TMVA::Types::kTraining );
      factory->AddBackgroundTree( TreeBSumTrain, 1.0, TMVA::Types::kTraining );
      factory->AddSignalTree    ( TreeSSumTest,  1.0, TMVA::Types::kTesting  );
      factory->AddBackgroundTree( TreeBSumTest, 1.0 , TMVA::Types::kTesting  );
      factory->SetBackgroundWeightExpression("weight");
      factory->PrepareTrainingAndTestTree( dummycut, dummycut,
                                           "nTrain_Signal=0:nTrain_Background=0:SplitMode=Block:NormMode=NumEvents:!V" );      
      thetest->RegisterAssertion(4000,4000,4000,4000);
   }
   else if (index==17){ 
      factory->AddSignalTree    ( TreeSSumTrain, 1.0, TMVA::Types::kTraining );
      factory->AddBackgroundTree( TreeBSumTrain, 1.0, TMVA::Types::kTraining );
      factory->AddSignalTree    ( TreeSSumTest,  1.0, TMVA::Types::kTesting  );
      factory->AddBackgroundTree( TreeBSumTest, 1.0 , TMVA::Types::kTesting  );
      factory->SetBackgroundWeightExpression("weight");
      factory->PrepareTrainingAndTestTree( dummycut, dummycut,
                                           "nTrain_Signal=3999:nTest_Background=3998:SplitMode=Block:NormMode=NumEvents:!V" );      
      std::cout << "strict separation of testing and training, splitmode=block, 3999,4000,4000,3998 events"<<std::endl;
      thetest->RegisterAssertion(3999,4000,4000,3998);
   }
   else if (index==18){ 
      factory->AddSignalTree    ( TreeS1Train, 1.0, TMVA::Types::kTraining );
      factory->AddSignalTree    ( TreeS2Train, 1.0, TMVA::Types::kTraining );
      factory->AddBackgroundTree( TreeBSumTrain, 1.0, TMVA::Types::kTraining );
      factory->AddSignalTree    ( TreeSSumTest,  1.0, TMVA::Types::kTesting  );
      factory->AddBackgroundTree( TreeBSumTest, 1.0 , TMVA::Types::kTesting  );
      factory->SetBackgroundWeightExpression("weight");
      factory->PrepareTrainingAndTestTree( dummycut,
                                           "nTrain_Signal=4000:nTrain_Background=4000:nTest_Signal=4000:nTest_Background=4000:SplitMode=Block:NormMode=NumEvents:!V" );      
      std::cout << "strict separation of testing and training, 8000 train 8000 test total, using PrepareTraining.. with one cut, splitmode=block"<<std::endl;
      thetest->RegisterAssertion(4000,4000,4000,4000);
   }   
   // index = 20+ one of the event numbers is 0
   else if (index==20) {
      std::cout << "strict separation, 5000 train, 4500 test total, bckground booked with 0 - meaning take half"<<std::endl;
      factory->AddSignalTree    ( TreeSSumTrain, 1.0, TMVA::Types::kTraining );
      factory->AddBackgroundTree( TreeBSumTrain, 1.0, TMVA::Types::kTraining );
      factory->AddSignalTree    ( TreeSSumTest,  1.0, TMVA::Types::kTesting  );
      factory->AddBackgroundTree( TreeBSumTest,  1.0, TMVA::Types::kTesting  );
      factory->SetBackgroundWeightExpression("weight");
      factory->PrepareTrainingAndTestTree( dummycut, dummycut,
                                           "nTrain_Signal=1000:nTrain_Background=0:nTest_Signal=500:nTest_Background=0:SplitMode=Random:NormMode=NumEvents:!V" );  
      thetest->RegisterAssertion(1000,4000,500,4000);
   }
   // index= 30+ devoted to AddTree(TTree* tree, const TString& className, Double_t weight,const TCut& cut, const TString& treetype )
   else if (index==21){ 
      factory->AddSignalTree    ( TreeSSumTrain, 1.0, TMVA::Types::kTraining );
      factory->AddBackgroundTree( TreeBSumTrain, 1.0, TMVA::Types::kTraining );
      factory->AddSignalTree    ( TreeSSumTest,  1.0, TMVA::Types::kTesting  );
      factory->AddBackgroundTree( TreeBSumTest, 1.0 , TMVA::Types::kTesting  );
      factory->SetBackgroundWeightExpression("weight");
      factory->PrepareTrainingAndTestTree( dummycut, dummycut,
                                           "nTrain_Signal=4001:nTrain_Background=4000:SplitMode=Random:NormMode=NumEvents:!V" );      
      std::cout << "requesting more events than available, should cause a fatal, using Nevts_test=0, splitmode=random"<<std::endl;
      thetest->RegisterAssertion(4001,4000,4000,4000);
   }
   else if (index==22){ 
      factory->AddSignalTree    ( TreeSSumTrain, 1.0, TMVA::Types::kTraining );
      factory->AddBackgroundTree( TreeBSumTrain, 1.0, TMVA::Types::kTraining );
      factory->AddSignalTree    ( TreeSSumTest,  1.0, TMVA::Types::kTesting  );
      factory->AddBackgroundTree( TreeBSumTest, 1.0 , TMVA::Types::kTesting  );
      factory->SetBackgroundWeightExpression("weight");
      factory->PrepareTrainingAndTestTree( dummycut, dummycut,
                                           "nTest_Signal=4001:nTest_Background=4000:SplitMode=Random:NormMode=NumEvents:!V" );      
      std::cout << "requesting more events than available, should cause a fatal, using Nevts_train=0, splitmode=random"<<std::endl;
      thetest->RegisterAssertion(4000,4000,4001,4000);
   }
   else if (index==23){ 
      factory->AddSignalTree    ( TreeSSumTrain, 1.0, TMVA::Types::kTraining );
      factory->AddBackgroundTree( TreeBSumTrain, 1.0, TMVA::Types::kTraining );
      factory->AddSignalTree    ( TreeSSumTest,  1.0, TMVA::Types::kTesting  );
      factory->AddBackgroundTree( TreeBSumTest, 1.0 , TMVA::Types::kTesting  );
      factory->SetBackgroundWeightExpression("weight");
      factory->PrepareTrainingAndTestTree( dummycut, dummycut,
                                           "nTest_Signal=4001:nTest_Background=4001:SplitMode=Random:NormMode=NumEvents:!V" );      
      std::cout << "requesting more events than available, should cause a fatal, using Nevts_train=0, splitmode=random"<<std::endl;
      thetest->RegisterAssertion(4000,4000,4001,4000);
   }
   else if (index==24){ 
      factory->AddSignalTree    ( TreeSSumTrain, 1.0, TMVA::Types::kTraining );
      factory->AddBackgroundTree( TreeBSumTrain, 1.0, TMVA::Types::kTraining );
      factory->AddSignalTree    ( TreeSSumTest,  1.0, TMVA::Types::kTesting  );
      factory->AddBackgroundTree( TreeBSumTest, 1.0 , TMVA::Types::kTesting  );
      factory->SetBackgroundWeightExpression("weight");
      factory->PrepareTrainingAndTestTree( dummycut, dummycut,
                                           "nTrain_Signal=4000:nTrain_Background=4000:nTest_Signal=6000:nTest_Background=6000:SplitMode=Random:NormMode=NumEvents:!V" );      
      std::cout << "requesting more events than available, should cause a fatal, not a broken assertion"<<std::endl;
      thetest->RegisterAssertion(4000,4000,6000,6000);
   }
   else if (index==30) {
      std::cout << "strict separation, 3000 train, 2000 test total, using AddTree(TTree* tree, const TString& className, Double_t weight,const TCut& cut, const TString& treetype )"<<std::endl;
      factory->AddTree( TreeSSumTrain, "Signal", 1.0, "", "Training" );
      factory->AddTree( TreeSSumTest, "Signal", 1.0, "", "Testing" );
      factory->AddTree( TreeB1Train, "Background", 1.0, "", "Training" );
      factory->AddTree( TreeB2Test, "Background", 1.0, "", "Testing" );
      factory->SetBackgroundWeightExpression("weight");
      factory->PrepareTrainingAndTestTree( dummycut, dummycut,
                                           "nTrain_Signal=1000:nTrain_Background=2000:nTest_Signal=1000:nTest_Background=1000:SplitMode=Random:NormMode=NumEvents:!V" );  
      thetest->RegisterAssertion(1000,2000,1000,1000);
   }
   else if (index==31) {
      std::cout << "strict separation, 3000 train, 2000 test total, using AddTree(TTree* tree, const TString& className, Double_t weight,const TCut& cut, const TString& treetype )"<<std::endl;
      factory->AddTree( TreeSSumTrain, "Signal", 1.0, "", "Training" );
      factory->AddTree( TreeSSumTest, "Signal", 1.0, "", "Testing" );
      factory->AddTree( TreeB1Train, "Background", 1.0, "", "Training" );
      factory->AddTree( TreeB2Test, "Background", 1.0, "", "Testing" );
      factory->SetBackgroundWeightExpression("weight");
      factory->PrepareTrainingAndTestTree( dummycut, dummycut,
                                           "nTrain_Signal=1000:nTrain_Background=0:nTest_Signal=1000:nTest_Background=1000:SplitMode=Random:NormMode=NumEvents:!V" );  
      thetest->RegisterAssertion(1000,2000,1000,1000);
   }
   else if (index==32) {
      std::cout << "strict separation, 3000 train, 2000 test total, using AddTree(TTree* tree, const TString& className, Double_t weight,const TCut& cut, const TString& treetype ), try AddTree with cuts"<<std::endl;
      factory->AddTree( TreeSSumTrain, "Signal", 1.0, "", "Training" );
      factory->AddTree( TreeSSumTest, "Signal", 1.0, "", "Testing" );
      factory->AddTree( TreeB1Train, "Background", 1.0, "", "Training" );
      factory->AddTree( TreeB2Test, "Background", 1.0, "var2>-2.", "Testing" );
      factory->SetBackgroundWeightExpression("weight");
      factory->PrepareTrainingAndTestTree( dummycut, dummycut,
                                           "nTrain_Signal=1000:nTrain_Background=0:nTest_Signal=1000:nTest_Background=1000:SplitMode=Random:NormMode=NumEvents:!V" );  
      thetest->RegisterAssertion(1000,2000,1000,1000);
   }
   else if (index==33) {
      std::cout << "strict separation, 3000 train, 2000 test total, using AddTree(TTree* tree, const TString& className, Double_t weight,const TCut& cut, const TString& treetype ), try AddTree with cuts"<<std::endl;
      factory->AddTree( TreeSSumTrain, "Signal", 1.0, "", "Training" );
      factory->AddTree( TreeSSumTest, "Signal", 1.0, "", "Testing" );
      factory->AddTree( TreeB1Train, "Background", 1.0, "", "Training" );
      factory->AddTree( TreeB2Test, "Background", 1.0, "var2>-2.", "Testing" );
      factory->SetBackgroundWeightExpression("weight");
      factory->PrepareTrainingAndTestTree( dummycut, dummycut,
                                           "nTrain_Signal=1000:nTrain_Background=1000:nTest_Signal=1000:nTest_Background=1000:SplitMode=Random:NormMode=NumEvents:!V" );  
      thetest->RegisterAssertion(1000,1000,1000,1000);
   }
   // index= 40+ devoted to AddTree(TTree* tree, const TString& className, Double_t weight, const TCut& cut, Types::ETreeType tt )
   else if (index==40) {
      std::cout << "strict separation, 3000 train, 2000 test total, using AddTree(TTree* tree, const TString& className, Double_t weight, const TCut& cut, Types::ETreeType tt )"<<std::endl;
      factory->AddTree( TreeSSumTrain, "Signal",1.0, "",TMVA::Types::kTraining);
      factory->AddTree( TreeSSumTest, "Signal", 1.0, "",TMVA::Types::kTesting);
      factory->AddTree( TreeB1Train, "Background", 1.0, "", TMVA::Types::kTraining);
      factory->AddTree( TreeB2Test, "Background", 1.0, "", TMVA::Types::kTesting );
      factory->SetBackgroundWeightExpression("weight");
      factory->PrepareTrainingAndTestTree( dummycut, dummycut,
                                           "nTrain_Signal=1000:nTrain_Background=2000:nTest_Signal=1000:nTest_Background=1000:SplitMode=Random:NormMode=NumEvents:!V" );  
      thetest->RegisterAssertion(1000,2000,1000,1000);
   }
   // index= 40+ devoted to AddTree(TTree* tree, const TString& className, Double_t weight, const TCut& cut, Types::ETreeType tt )
   else if (index==41) {
      std::cout << "strict separation, 6000 train, 2000 test total, using AddTree(TTree* tree, const TString& className, Double_t weight, const TCut& cut, Types::ETreeType tt )"<<std::endl;
      factory->AddTree( TreeSSumTrain, "Signal",1.0, "",TMVA::Types::kTraining);
      factory->AddTree( TreeSSumTest, "Signal", 1.0, "",TMVA::Types::kTesting);
      factory->AddTree( TreeB1Train, "Background", 1.0, "", TMVA::Types::kTraining);
      factory->AddTree( TreeB2Test, "Background", 1.0, "", TMVA::Types::kTesting );
      factory->SetBackgroundWeightExpression("weight");
      factory->PrepareTrainingAndTestTree( dummycut, dummycut,
                                           "nTrain_Signal=0:nTrain_Background=2000:nTest_Signal=1000:nTest_Background=1000:SplitMode=Random:NormMode=NumEvents:!V" );  
      thetest->RegisterAssertion(4000,2000,1000,1000);
   }
   // index= 40+ devoted to AddTree(TTree* tree, const TString& className, Double_t weight, const TCut& cut, Types::ETreeType tt )
   else if (index==42) {
      std::cout << "strict separation, 3000 train, 2000 test total, using AddTree(TTree* tree, const TString& className, Double_t weight, const TCut& cut, Types::ETreeType tt ), try AdTree with cuts"<<std::endl;
      factory->AddTree( TreeSSumTrain, "Signal",1.0, "",TMVA::Types::kTraining);
      factory->AddTree( TreeSSumTest, "Signal", 1.0, "",TMVA::Types::kTesting);
      factory->AddTree( TreeB1Train, "Background", 1.0, "", TMVA::Types::kTraining);
      factory->AddTree( TreeB2Test, "Background", 1.0, "var2>-1.5", TMVA::Types::kTesting );
      factory->SetBackgroundWeightExpression("weight");
      factory->PrepareTrainingAndTestTree( dummycut, dummycut,
                                           "nTrain_Signal=1000:nTrain_Background=2000:nTest_Signal=1000:nTest_Background=1000:SplitMode=Random:NormMode=NumEvents:!V" );  
      thetest->RegisterAssertion(1000,2000,1000,1000);
   }

   // index= 50+ devoted to SetSignal/Background tree (are these deprecated functions?)
   else if (index==50) {
      std::cout << "no strict separation between test/training, 3000 train, 2000 test total, using SetSignalTree( TTree* tree, Double_t weight )"<<std::endl;
      factory->SetSignalTree( TreeSSumTest, 1.);
      factory->SetBackgroundTree( TreeBSumTest, 1.);
      factory->SetBackgroundWeightExpression("weight");
      factory->PrepareTrainingAndTestTree( dummycut, dummycut,
                                           "nTrain_Signal=1000:nTrain_Background=2000:nTest_Signal=1000:nTest_Background=1000:SplitMode=Random:NormMode=NumEvents:!V" );  
      thetest->RegisterAssertion(1000,2000,1000,1000,"testtrainmixed");
   }
   else if (index==51) {
      std::cout << "no strict separation between test/training, 5000 train, 2000 test total, using SetSignalTree( TTree* tree, Double_t weight ) and nTrain_Signal=0"<<std::endl;
      factory->SetSignalTree( TreeSSumTest, 1.);
      factory->SetBackgroundTree( TreeBSumTest, 1.);
      factory->SetBackgroundWeightExpression("weight");
      factory->PrepareTrainingAndTestTree( dummycut, dummycut,
                                           "nTrain_Signal=0:nTrain_Background=2000:nTest_Signal=1000:nTest_Background=1000:SplitMode=Random:NormMode=NumEvents:!V" );  
      thetest->RegisterAssertion(3000,2000,1000,1000,"testtrainmixed");
   }
   // index= 60+ devoted to addition of single events
   else if (index==60) {
      std::cout << "strict separation of test/training, 2000 train, 2000 test total, using  AddEvent        ( const TString& className, Types::ETreeType tt, const std::vector<Double_t>& event, Double_t weight ); for test and training"<<std::endl;
      std::vector<Float_t> fevent;
      std::vector<Double_t> devent;
      TTree* tree = TreeSSumTrain;
      PrepareVector(tree, fevent);
      for (int i=0;i<1000;i++){
         tree->GetEntry(i);
         CopyToDouble(fevent,devent);
         factory->AddEvent( TString("Signal") , TMVA::Types::kTraining, devent, 1.);
      }
      tree = TreeBSumTrain;
      PrepareVector(tree, fevent);
      for (int i=0;i<1000;i++){
         tree->GetEntry(i);
         CopyToDouble(fevent,devent);
         if (i<10) std::cout << "i="<<i<<" var1="<<devent[0]<<" var2="<<devent[1]<<std::endl;
         factory->AddEvent( TString("Background") , TMVA::Types::kTraining, devent, 1.);
      }
      tree = TreeSSumTest;
      PrepareVector(tree, fevent);
      for (int i=0;i<1000;i++){
         tree->GetEntry(i);
         CopyToDouble(fevent,devent);
         factory->AddEvent( TString("Signal") , TMVA::Types::kTesting, devent, 1.);
      }
      tree = TreeBSumTest;
      PrepareVector(tree, fevent);
      for (int i=0;i<1000;i++){
         tree->GetEntry(i);
         CopyToDouble(fevent,devent);
         factory->AddEvent( TString("Background") , TMVA::Types::kTesting, devent, 1.);
      }
      factory->PrepareTrainingAndTestTree( dummycut, dummycut,
                                           "nTrain_Signal=1000:nTrain_Background=1000:nTest_Signal=1000:nTest_Background=1000:SplitMode=Random:NormMode=NumEvents:!V" );  
      thetest->RegisterAssertion(1000,1000,1000,1000);
   }
   // index= 70+ devoted to ascii files 
   else if (index==70){ 
      std::cout << "using ascii files and function SetInputTrees( const TString& signalFileName, const TString& backgroundFile, signalWeight=1.0, backgroundWeight=1.0 );"<<std::endl;
      factory->SetInputTrees("fileSSumTest.dat","fileBSumTest.dat");
      factory->PrepareTrainingAndTestTree( dummycut, dummycut,
                                           "nTrain_Signal=1000:nTrain_Background=1001:nTest_Signal=1002:nTest_Background=1003:SplitMode=Random:NormMode=NumEvents:!V" );
      thetest->RegisterAssertion(1000,1001,1002,1003,"testtrainmixed");
   }
   else if (index==71){ 
      std::cout << "using ascii files and function AddSignalTree( datfile ...);"<<std::endl;
      factory->AddSignalTree("fileSSumTest.dat",1.0,TMVA::Types::kTesting);
      factory->AddBackgroundTree("fileBSumTest.dat",1.0,TMVA::Types::kTesting);
      factory->AddSignalTree("fileSSumTrain.dat",1.0,TMVA::Types::kTraining);
      factory->AddBackgroundTree("fileBSumTrain.dat",1.0,TMVA::Types::kTraining);
      factory->PrepareTrainingAndTestTree( dummycut, dummycut,
                                           "nTrain_Signal=1000:nTrain_Background=1001:nTest_Signal=1002:nTest_Background=1003:SplitMode=Random:NormMode=NumEvents:!V" );
      thetest->RegisterAssertion(1000,1001,1002,1003);
   }
   // the 100+ tests are devoted to insertion of fake events and their correct handling/detection or to the intentional mixing of training and test events
   else if (index==100){ 
      std::cout << "using test tree for testing and training, 4000 train 4000 test total, using one S+B tree and SetInputTrees"<<std::endl;
      factory->SetInputTrees(TreeSBSumTrain, "issig>0.5", "issig<0.5"); //tree signalcut, BG cut
      factory->SetBackgroundWeightExpression("weight");
      factory->PrepareTrainingAndTestTree( dummycut, dummycut,
                                           "nTrain_Signal=2000:nTrain_Background=2000:nTest_Signal=2000:nTest_Background=2000:SplitMode=Random:NormMode=NumEvents:!V" );      
      thetest->RegisterAssertion(2000,2000,2000,2000,"testtrainmixed");
   }
   else if (index==101){ 
      std::cout << "using test tree for testing and training, 4000 train 4000 test total, using one S+B tree and SetInputTrees, using one-cut PrepareTr.."<<std::endl;
      factory->SetInputTrees(TreeSBSumTrain, "issig>0.5", "issig<0.5"); //tree signalcut, BG cut
      factory->SetBackgroundWeightExpression("weight");
      factory->PrepareTrainingAndTestTree( dummycut,
                                           "nTrain_Signal=2000:nTrain_Background=2000:nTest_Signal=2000:nTest_Background=2000:SplitMode=Random:NormMode=NumEvents:!V" );      
      thetest->RegisterAssertion(2000,2000,2000,2000,"testtrainmixed");
   }
   else if (index==102){ 
      std::cout << "using test tree for testing and training, 3000 train 2000 test total, using one S+B tree and SetInputTrees"<<std::endl;
      factory->SetInputTrees(TreeSBSumTrain, "issig>0.5", "issig<0.5"); //tree signalcut, BG cut
      factory->SetBackgroundWeightExpression("weight");
      factory->PrepareTrainingAndTestTree( dummycut, dummycut,
                                           "nTrain_Signal=1000:nTrain_Background=2000:nTest_Signal=1000:nTest_Background=1000:SplitMode=Random:NormMode=NumEvents:!V" );      
      thetest->RegisterAssertion(1000,2000,1000,1000,"testtrainmixed");
   }
   else if (index==103){ 
      std::cout << "using test tree for testing and training, 3000 train 2000 test total, using one S+B tree and SetInputTrees, try condition issig==1"<<std::endl;
      factory->SetInputTrees(TreeSBSumTrain, "issig==1", "issig==0"); //tree signalcut, BG cut
      factory->SetBackgroundWeightExpression("weight");
      factory->PrepareTrainingAndTestTree( dummycut, dummycut,
                                           "nTrain_Signal=1000:nTrain_Background=2000:nTest_Signal=1000:nTest_Background=1000:SplitMode=Random:NormMode=NumEvents:!V" );      
      thetest->RegisterAssertion(1000,2000,1000,1000,"testtrainmixed");
   }
   else if (index==104){ 
      std::cout << "using test tree for testing and training, 3000 train 2000 test total, using one S+B tree and SetInputTrees, try signal cut in PrepraeTR.."<<std::endl;
      factory->SetInputTrees(TreeSBSumTrain, "issig==1", "issig==0"); //tree signalcut, BG cut
      factory->SetBackgroundWeightExpression("weight");
      factory->PrepareTrainingAndTestTree( "var2>-2.", dummycut,
                                           "nTrain_Signal=1000:nTrain_Background=2000:nTest_Signal=1000:nTest_Background=1000:SplitMode=Random:NormMode=NumEvents:!V" );      
      thetest->RegisterAssertion(1000,2000,1000,1000,"testtrainmixed");
   }
   else if (index==105){ 
      std::cout << "using train tree for testing and training, 3000 train 2000 test total, using SetInputTrees and two trees"<<std::endl;
      factory->SetInputTrees(TreeSSumTrain, TreeBSumTrain); 
      factory->SetBackgroundWeightExpression("weight");
      factory->PrepareTrainingAndTestTree( dummycut, dummycut,
                                           "nTrain_Signal=1000:nTrain_Background=2000:nTest_Signal=1000:nTest_Background=1000:SplitMode=Random:NormMode=NumEvents:!V" );      
      thetest->RegisterAssertion(1000,2000,1000,1000,"testtrainmixed");
   }
   else if (index==106){ 
      std::cout << "using train tree for testing and training, 3000 train 2000 test total, using SetInputTrees and two trees, try cut in PreprareTr..."<<std::endl;
      factory->SetInputTrees(TreeSSumTrain, TreeBSumTrain); 
      factory->SetBackgroundWeightExpression("weight");
      factory->PrepareTrainingAndTestTree( "var2>-2.", dummycut,
                                           "nTrain_Signal=1000:nTrain_Background=2000:nTest_Signal=1000:nTest_Background=1000:SplitMode=Random:NormMode=NumEvents:!V" );      
      thetest->RegisterAssertion(1000,2000,1000,1000,"testtrainmixed");
   }
   else if (index==107){ 
      std::cout << "using test tree for testing and training, 4000 train 4000 test total, using one S+B tree and SetInputTrees"<<std::endl;
      factory->SetInputTrees(TreeSBSumTrain, "issig>0.5", "issig<0.5"); //tree signalcut, BG cut
      factory->SetBackgroundWeightExpression("weight");
      factory->PrepareTrainingAndTestTree( dummycut, dummycut,
                                           "nTrain_Signal=0:nTrain_Background=0:nTest_Signal=0:nTest_Background=0:SplitMode=Random:NormMode=NumEvents:!V" );      
      thetest->RegisterAssertion(2000,2000,2000,2000,"testtrainmixed");
   }
   else if (index==108){ 
      std::cout << "using test tree for testing and training, 4000 train 4000 test total, using one S+B tree and SetInputTrees"<<std::endl;
      factory->SetInputTrees(TreeSBSumTrain, "issig>0.5", "issig<0.5"); //tree signalcut, BG cut
      factory->AddTree( TreeB2Test, "Background", 1.0, "", "Testing" );
      factory->SetBackgroundWeightExpression("weight");
      factory->PrepareTrainingAndTestTree( dummycut, dummycut,
                                           "nTrain_Signal=0:nTrain_Background=0:nTest_Signal=0:nTest_Background=0:SplitMode=Random:NormMode=NumEvents:!V" );      
      thetest->RegisterAssertion(2000,3000,2000,3000,"testtrainmixed");
   }
   else if (index==109){ 
      std::cout << "using a large train tree for testing and training, 5000 train 5000 test total, using one S+B tree and SetInputTrees"<<std::endl;
      factory->SetInputTrees(TreeSBLarge, "issig>0.5", "issig<0.5"); //tree signalcut, BG cut
      factory->SetBackgroundWeightExpression("weight");
      factory->PrepareTrainingAndTestTree( dummycut, dummycut,
                                           "nTrain_Signal=5000:nTrain_Background=5000:nTest_Signal=5000:nTest_Background=5000:SplitMode=Random:NormMode=NumEvents:!V" );      
      thetest->RegisterAssertion(5000,5000,5000,5000,"testtrainmixed");
   }

   else if (index==110) {
      std::cout << "strict separation, 8000 train, 8000 test total, adding fake trees to testing which are not used because of block mode"<<std::endl;
      factory->AddSignalTree    ( TreeSSumTrain, 1.0, TMVA::Types::kTraining );
      factory->AddBackgroundTree( TreeBSumTrain, 1.0, TMVA::Types::kTraining );
      factory->AddSignalTree    ( TreeSSumTest,  1.0, TMVA::Types::kTesting  );
      factory->AddBackgroundTree( TreeBSumTest,  1.0, TMVA::Types::kTesting  );
      factory->AddSignalTree    ( TreeSFake, 1.0, TMVA::Types::kTesting );
      factory->AddBackgroundTree( TreeBFake, 1.0, TMVA::Types::kTesting );
      factory->SetBackgroundWeightExpression("weight");
      factory->PrepareTrainingAndTestTree( dummycut, dummycut,
                                           "nTrain_Signal=4000:nTrain_Background=4000:nTest_Signal=4000:nTest_Background=4000:SplitMode=Block:NormMode=NumEvents:!V" );  
      thetest->RegisterAssertion(4000,4000,4000,4000);
   }
   else if (index==111) {
      std::cout << "strict separation, 8000 train, 9000 test total, adding fake trees to testing"<<std::endl;
      factory->AddSignalTree    ( TreeSSumTrain, 1.0, TMVA::Types::kTraining );
      factory->AddBackgroundTree( TreeBSumTrain, 1.0, TMVA::Types::kTraining );
      factory->AddSignalTree    ( TreeSSumTest,  1.0, TMVA::Types::kTesting  );
      factory->AddBackgroundTree( TreeBSumTest,  1.0, TMVA::Types::kTesting  );
      factory->AddSignalTree    ( TreeSFake, 1.0, TMVA::Types::kTesting );
      factory->AddBackgroundTree( TreeBFake, 1.0, TMVA::Types::kTesting );
      factory->SetBackgroundWeightExpression("weight");
      factory->PrepareTrainingAndTestTree( dummycut, dummycut,
                                           "nTrain_Signal=4000:nTrain_Background=4000:nTest_Signal=4000:nTest_Background=5000:SplitMode=Random:NormMode=NumEvents:!V" );  
      thetest->RegisterAssertion(4000,4000,4000,5000,"hastestfakes");
   }
   else if (index==120) {
      std::cout << "strict separation, 8000 train, 8000 test total, adding fake trees to testing which diffuse into testing sample via random mixing"<<std::endl;
      factory->AddSignalTree    ( TreeSSumTrain, 1.0, TMVA::Types::kTraining );
      factory->AddBackgroundTree( TreeBSumTrain, 1.0, TMVA::Types::kTraining );
      factory->AddSignalTree    ( TreeSSumTest,  1.0, TMVA::Types::kTesting  );
      factory->AddBackgroundTree( TreeBSumTest,  1.0, TMVA::Types::kTesting  );
      factory->AddSignalTree    ( TreeSFake, 1.0, TMVA::Types::kTesting );
      factory->AddBackgroundTree( TreeBFake, 1.0, TMVA::Types::kTesting );
      factory->SetBackgroundWeightExpression("weight");
      factory->PrepareTrainingAndTestTree( dummycut, dummycut,
                                           "nTrain_Signal=4000:nTrain_Background=4000:nTest_Signal=4000:nTest_Background=4000:SplitMode=Random:NormMode=NumEvents:!V" );  
      thetest->RegisterAssertion(4000,4000,4000,4000,"hastestfakes");
   }
   else if (index==121) {
      std::cout << "strict separation, 8000 train, 8000 test total, adding fake trees to testing which diffuse into testing sample via random mixing, reverse order of test and train"<<std::endl;
      factory->AddSignalTree    ( TreeSSumTest,  1.0, TMVA::Types::kTesting  );
      factory->AddBackgroundTree( TreeBSumTest,  1.0, TMVA::Types::kTesting  );
      factory->AddSignalTree    ( TreeSSumTrain, 1.0, TMVA::Types::kTraining );
      factory->AddBackgroundTree( TreeBSumTrain, 1.0, TMVA::Types::kTraining );
      factory->AddSignalTree    ( TreeSFake, 1.0, TMVA::Types::kTesting );
      factory->AddBackgroundTree( TreeBFake, 1.0, TMVA::Types::kTesting );
      factory->SetBackgroundWeightExpression("weight");
      factory->PrepareTrainingAndTestTree( dummycut, dummycut,
                                           "nTrain_Signal=4000:nTrain_Background=4000:nTest_Signal=4000:nTest_Background=4000:SplitMode=Random:NormMode=NumEvents:!V" );  
      thetest->RegisterAssertion(4000,4000,4000,4000,"hastestfakes");
   }
   else if (index==122) {
      std::cout << "strict separation, 8000 train, 8000 test total, adding fake trees to testing which are not used because of alternate mode"<<std::endl;
      factory->AddSignalTree    ( TreeSSumTrain, 1.0, TMVA::Types::kTraining );
      factory->AddBackgroundTree( TreeBSumTrain, 1.0, TMVA::Types::kTraining );
      factory->AddSignalTree    ( TreeSSumTest,  1.0, TMVA::Types::kTesting  );
      factory->AddBackgroundTree( TreeBSumTest,  1.0, TMVA::Types::kTesting  );
      factory->AddSignalTree    ( TreeSFake, 1.0, TMVA::Types::kTesting );
      factory->AddBackgroundTree( TreeBFake, 1.0, TMVA::Types::kTesting );
      factory->SetBackgroundWeightExpression("weight");
      factory->PrepareTrainingAndTestTree( dummycut, dummycut,
                                           "nTrain_Signal=4000:nTrain_Background=4000:nTest_Signal=4000:nTest_Background=4000:SplitMode=Alternate:NormMode=NumEvents:!V" );  
      thetest->RegisterAssertion(4000,4000,4000,4000);
   }
   else if (index==123) {
      std::cout << "strict separation, 8000 train, 8000 test total, adding fake trees to testing which are not used because of alternate mode, uneven event numbers with alternate mode"<<std::endl;
      factory->AddSignalTree    ( TreeSSumTrain, 1.0, TMVA::Types::kTraining );
      factory->AddBackgroundTree( TreeBSumTrain, 1.0, TMVA::Types::kTraining );
      factory->AddSignalTree    ( TreeSSumTest,  1.0, TMVA::Types::kTesting  );
      factory->AddBackgroundTree( TreeBSumTest,  1.0, TMVA::Types::kTesting  );
      factory->AddSignalTree    ( TreeSFake, 1.0, TMVA::Types::kTesting );
      factory->SetBackgroundWeightExpression("weight");
      factory->PrepareTrainingAndTestTree( dummycut, dummycut,
                                           "nTrain_Signal=4000:nTrain_Background=4000:nTest_Signal=4000:nTest_Background=3000:SplitMode=Alternate:NormMode=NumEvents:!V" );  
      thetest->RegisterAssertion(4000,4000,4000,3000);
   }
   else {
      std::cout <<" unknown index mode exiting"<<std::endl;
      exit(1);
   }

}

//_______________________________________________________________________
int main( int argc, char** argv ) 
{
   //---------------------------------------------------------------
   // default MVA methods to be trained + tested
   std::map<std::string,int> Use;

   Use["Cuts"]            = 0;
   Use["CutsD"]           = 0;
   Use["CutsPCA"]         = 0;
   Use["CutsGA"]          = 0;
   Use["CutsSA"]          = 0;
   // ---
   Use["Likelihood"]      = 0;
   Use["LikelihoodD"]     = 0; // the "D" extension indicates decorrelated input variables (see option strings)
   Use["LikelihoodPCA"]   = 0; // the "PCA" extension indicates PCA-transformed input variables (see option strings)
   Use["LikelihoodKDE"]   = 0;
   Use["LikelihoodMIX"]   = 0;
   // ---
   Use["PDERS"]           = 0;
   Use["PDERSD"]          = 0;
   Use["PDERSPCA"]        = 0;
   Use["PDERSkNN"]        = 0; // depreciated until further notice
   Use["PDEFoam"]         = 0;
   // --
   Use["KNN"]             = 0;
   // ---
   Use["HMatrix"]         = 0;
   Use["Fisher"]          = 1;
   Use["FisherG"]         = 0;
   Use["BoostedFisher"]   = 0;
   Use["LD"]              = 1;
   // ---
   Use["FDA_GA"]          = 0;
   Use["FDA_SA"]          = 0;
   Use["FDA_MC"]          = 0;
   Use["FDA_MT"]          = 0;
   Use["FDA_GAMT"]        = 0;
   Use["FDA_MCMT"]        = 0;
   // ---
   Use["MLP"]             = 0; // this is the recommended ANN
   Use["MLPBFGS"]         = 0; // recommended ANN with optional training method
   Use["CFMlpANN"]        = 0; // *** missing
   Use["TMlpANN"]         = 0; 
   // ---
   Use["SVM"]             = 0;
   // ---
   Use["BDT"]             = 0;
   Use["BDTD"]            = 0;
   Use["BDTG"]            = 0;
   Use["BDTB"]            = 0;
   // ---
   Use["RuleFit"]         = 0;
   // ---------------------------------------------------------------

   std::cout << std::endl << "==> Start TMVAClassification" << std::endl;

   bool batchMode(false);
   bool useDefaultMethods(true);
   int indx=0;
   if (argc>1) indx = atoi(argv[1]);

   // Create a new root output file.
   TString tmvaname = Form("TMVATestDataInput%d",indx);
   TString outfileName = tmvaname + TString(".root");
   TFile* outputFile = TFile::Open( outfileName, "RECREATE" );

   // Create the factory object.
   std::string factoryOptions( "!V:!Silent:Transformations=I;D;P;G,D" );
   if (batchMode) factoryOptions += ":!Color:!DrawProgressBar";

   TMVA::Factory *factory = new TMVA::Factory( tmvaname, outputFile, factoryOptions );

   // Define the input variables that shall be used for the MVA training
   factory->AddVariable( "var1",  "Variable 1", "units", 'F' );
   factory->AddVariable( "var2",  "Variable 2", "units", 'F' );
   factory->AddVariable( "var3",  "Variable 3", "units", 'F' );
   factory->AddVariable( "var4",  "Variable 4", "units", 'F' );
   // Spectator variables
   factory->AddSpectator( "issig");  // =1 if signal
   factory->AddSpectator( "istest"); // =1 if an event meant for tests
   factory->AddSpectator( "isfake"); // =1 if the event was expected to be excluded from test or training

   // read training and test data, index number describes method used to pass data to TMVA
   DataInputTest* thetest=new DataInputTest();

   SetInputData(indx, factory, thetest); //

   // ---- Book MVA methods
   // Cut optimisation
   if (Use["Cuts"])
      factory->BookMethod( TMVA::Types::kCuts, "Cuts", 
                           "!H:!V:FitMethod=MC:EffSel:SampleSize=200000:VarProp=FSmart" );

   if (Use["CutsD"])
      factory->BookMethod( TMVA::Types::kCuts, "CutsD", 
                           "!H:!V:FitMethod=MC:EffSel:SampleSize=200000:VarProp=FSmart:VarTransform=Decorrelate" );

   if (Use["CutsPCA"])
      factory->BookMethod( TMVA::Types::kCuts, "CutsPCA", 
                           "!H:!V:FitMethod=MC:EffSel:SampleSize=200000:VarProp=FSmart:VarTransform=PCA" );

   if (Use["CutsGA"])
      factory->BookMethod( TMVA::Types::kCuts, "CutsGA",
                           "H:!V:FitMethod=GA:CutRangeMin[0]=-10:CutRangeMax[0]=10:VarProp[1]=FMax:EffSel:Steps=30:Cycles=3:PopSize=400:SC_steps=10:SC_rate=5:SC_factor=0.95" );
   
   if (Use["CutsSA"])
      factory->BookMethod( TMVA::Types::kCuts, "CutsSA",
                           "!H:!V:FitMethod=SA:EffSel:MaxCalls=150000:KernelTemp=IncAdaptive:InitialTemp=1e+6:MinTemp=1e-6:Eps=1e-10:UseDefaultScale" );
   
   // Likelihood
   if (Use["Likelihood"])
      factory->BookMethod( TMVA::Types::kLikelihood, "Likelihood", 
                           "H:!V:TransformOutput:PDFInterpol=Spline2:NSmoothSig[0]=20:NSmoothBkg[0]=20:NSmoothBkg[1]=10:NSmooth=1:NAvEvtPerBin=50" ); 

   // test the decorrelated likelihood
   if (Use["LikelihoodD"])
      factory->BookMethod( TMVA::Types::kLikelihood, "LikelihoodD", 
                           "!H:!V:!TransformOutput:PDFInterpol=Spline2:NSmoothSig[0]=20:NSmoothBkg[0]=20:NSmooth=5:NAvEvtPerBin=50:VarTransform=Decorrelate" ); 

   if (Use["LikelihoodPCA"])
      factory->BookMethod( TMVA::Types::kLikelihood, "LikelihoodPCA", 
                           "!H:!V:!TransformOutput:PDFInterpol=Spline2:NSmoothSig[0]=20:NSmoothBkg[0]=20:NSmooth=5:NAvEvtPerBin=50:VarTransform=PCA" ); 
 
   // test the new kernel density estimator
   if (Use["LikelihoodKDE"])
      factory->BookMethod( TMVA::Types::kLikelihood, "LikelihoodKDE", 
                           "!H:!V:!TransformOutput:PDFInterpol=KDE:KDEtype=Gauss:KDEiter=Adaptive:KDEFineFactor=0.3:KDEborder=None:NAvEvtPerBin=50" ); 

   // test the mixed splines and kernel density estimator (depending on which variable)
   if (Use["LikelihoodMIX"])
      factory->BookMethod( TMVA::Types::kLikelihood, "LikelihoodMIX", 
                           "!H:!V:!TransformOutput:PDFInterpolSig[0]=KDE:PDFInterpolBkg[0]=KDE:PDFInterpolSig[1]=KDE:PDFInterpolBkg[1]=KDE:PDFInterpolSig[2]=Spline2:PDFInterpolBkg[2]=Spline2:PDFInterpolSig[3]=Spline2:PDFInterpolBkg[3]=Spline2:KDEtype=Gauss:KDEiter=Nonadaptive:KDEborder=None:NAvEvtPerBin=50" ); 

   // test the multi-dimensional probability density estimator
   // here are the options strings for the MinMax and RMS methods, respectively:
   //      "!H:!V:VolumeRangeMode=MinMax:DeltaFrac=0.2:KernelEstimator=Gauss:GaussSigma=0.3" );   
   //      "!H:!V:VolumeRangeMode=RMS:DeltaFrac=3:KernelEstimator=Gauss:GaussSigma=0.3" );   
   if (Use["PDERS"])
      factory->BookMethod( TMVA::Types::kPDERS, "PDERS", 
                           "!H:!V:NormTree=T:VolumeRangeMode=Adaptive:KernelEstimator=Gauss:GaussSigma=0.3:NEventsMin=400:NEventsMax=600" );

   if (Use["PDERSkNN"])
      factory->BookMethod( TMVA::Types::kPDERS, "PDERSkNN", 
                           "!H:!V:VolumeRangeMode=kNN:KernelEstimator=Gauss:GaussSigma=0.3:NEventsMin=400:NEventsMax=600" );

   if (Use["PDERSD"])
      factory->BookMethod( TMVA::Types::kPDERS, "PDERSD", 
                           "!H:!V:VolumeRangeMode=Adaptive:KernelEstimator=Gauss:GaussSigma=0.3:NEventsMin=400:NEventsMax=600:VarTransform=Decorrelate" );

   if (Use["PDERSPCA"])
      factory->BookMethod( TMVA::Types::kPDERS, "PDERSPCA", 
                           "!H:!V:VolumeRangeMode=Adaptive:KernelEstimator=Gauss:GaussSigma=0.3:NEventsMin=400:NEventsMax=600:VarTransform=PCA" );

   // Multi-dimensional likelihood estimator using self-adapting phase-space binning
   if (Use["PDEFoam"])
      factory->BookMethod( TMVA::Types::kPDEFoam, "PDEFoam", 
                           "H:!V:SigBgSeparate=F:TailCut=0.001:VolFrac=0.0333:nActiveCells=500:nSampl=2000:nBin=5:CutNmin=T:Nmin=100:Kernel=None:Compress=T" );

   // K-Nearest Neighbour classifier (KNN)
   if (Use["KNN"])
      factory->BookMethod( TMVA::Types::kKNN, "KNN", 
                           "H:nkNN=20:ScaleFrac=0.8:SigmaFact=1.0:Kernel=Gaus:UseKernel=F:UseWeight=T:!Trim" );
   // H-Matrix (chi2-squared) method
   if (Use["HMatrix"])
      factory->BookMethod( TMVA::Types::kHMatrix, "HMatrix", "!H:!V" ); 

   // Fisher discriminant   
   if (Use["Fisher"])
      factory->BookMethod( TMVA::Types::kFisher, "Fisher", "H:!V:Fisher:CreateMVAPdfs:PDFInterpolMVAPdf=Spline2:NbinsMVAPdf=60:NsmoothMVAPdf=10" );

   // Fisher with Gauss-transformed input variables
   if (Use["FisherG"])
      factory->BookMethod( TMVA::Types::kFisher, "FisherG", "H:!V:VarTransform=Gauss" );

   // Composite classifier: ensemble (tree) of boosted Fisher classifiers
   if (Use["BoostedFisher"])
      factory->BookMethod( TMVA::Types::kFisher, "BoostedFisher", "H:!V:Boost_Num=20:Boost_Transform=log:Boost_Type=AdaBoost:Boost_AdaBoostBeta=0.2");

   // Linear discriminant (same as Fisher)
   if (Use["LD"])
      factory->BookMethod( TMVA::Types::kLD, "LD", "H:!V:VarTransform=None" );

	// Function discrimination analysis (FDA) -- test of various fitters - the recommended one is Minuit (or GA or SA)
   if (Use["FDA_MC"])
      factory->BookMethod( TMVA::Types::kFDA, "FDA_MC",
                           "H:!V:Formula=(0)+(1)*x0+(2)*x1+(3)*x2+(4)*x3:ParRanges=(-1,1);(-10,10);(-10,10);(-10,10);(-10,10):FitMethod=MC:SampleSize=100000:Sigma=0.1" );
   
   if (Use["FDA_GA"]) // can also use Simulated Annealing (SA) algorithm (see Cuts_SA options])
      factory->BookMethod( TMVA::Types::kFDA, "FDA_GA",
                           "H:!V:Formula=(0)+(1)*x0+(2)*x1+(3)*x2+(4)*x3:ParRanges=(-1,1);(-10,10);(-10,10);(-10,10);(-10,10):FitMethod=GA:PopSize=300:Cycles=3:Steps=20:Trim=True:SaveBestGen=1" );

   if (Use["FDA_SA"]) // can also use Simulated Annealing (SA) algorithm (see Cuts_SA options])
      factory->BookMethod( TMVA::Types::kFDA, "FDA_SA",
                           "H:!V:Formula=(0)+(1)*x0+(2)*x1+(3)*x2+(4)*x3:ParRanges=(-1,1);(-10,10);(-10,10);(-10,10);(-10,10):FitMethod=SA:MaxCalls=15000:KernelTemp=IncAdaptive:InitialTemp=1e+6:MinTemp=1e-6:Eps=1e-10:UseDefaultScale" );

   if (Use["FDA_MT"])
      factory->BookMethod( TMVA::Types::kFDA, "FDA_MT",
                           "H:!V:Formula=(0)+(1)*x0+(2)*x1+(3)*x2+(4)*x3:ParRanges=(-1,1);(-10,10);(-10,10);(-10,10);(-10,10):FitMethod=MINUIT:ErrorLevel=1:PrintLevel=-1:FitStrategy=2:UseImprove:UseMinos:SetBatch" );

   if (Use["FDA_GAMT"])
      factory->BookMethod( TMVA::Types::kFDA, "FDA_GAMT",
                           "H:!V:Formula=(0)+(1)*x0+(2)*x1+(3)*x2+(4)*x3:ParRanges=(-1,1);(-10,10);(-10,10);(-10,10);(-10,10):FitMethod=GA:Converger=MINUIT:ErrorLevel=1:PrintLevel=-1:FitStrategy=0:!UseImprove:!UseMinos:SetBatch:Cycles=1:PopSize=5:Steps=5:Trim" );

   if (Use["FDA_MCMT"])
      factory->BookMethod( TMVA::Types::kFDA, "FDA_MCMT",
                           "H:!V:Formula=(0)+(1)*x0+(2)*x1+(3)*x2+(4)*x3:ParRanges=(-1,1);(-10,10);(-10,10);(-10,10);(-10,10):FitMethod=MC:Converger=MINUIT:ErrorLevel=1:PrintLevel=-1:FitStrategy=0:!UseImprove:!UseMinos:SetBatch:SampleSize=20" );

   // TMVA ANN: MLP (recommended ANN) -- all ANNs in TMVA are Multilayer Perceptrons
   if (Use["MLP"])
      factory->BookMethod( TMVA::Types::kMLP, "MLP", "H:!V:NeuronType=tanh:VarTransform=N:NCycles=600:HiddenLayers=N+5:TestRate=5" );

   if (Use["MLPBFGS"])
      factory->BookMethod( TMVA::Types::kMLP, "MLPBFGS", "H:!V:NeuronType=tanh:VarTransform=N:NCycles=600:HiddenLayers=N+5:TestRate=5:TrainingMethod=BFGS" );


   // CF(Clermont-Ferrand)ANN
   if (Use["CFMlpANN"])
      factory->BookMethod( TMVA::Types::kCFMlpANN, "CFMlpANN", "!H:!V:NCycles=2000:HiddenLayers=N+1,N"  ); // n_cycles:#nodes:#nodes:...  
  
   // Tmlp(Root)ANN
   if (Use["TMlpANN"])
      factory->BookMethod( TMVA::Types::kTMlpANN, "TMlpANN", "!H:!V:NCycles=200:HiddenLayers=N+1,N:LearningMethod=BFGS:ValidationFraction=0.3"  ); // n_cycles:#nodes:#nodes:...
  
   // Support Vector Machine
   if (Use["SVM"])
      factory->BookMethod( TMVA::Types::kSVM, "SVM", "Gamma=0.25:Tol=0.001:VarTransform=Norm" );
   
   // Boosted Decision Trees
   if (Use["BDTG"]) // Gradient Boost
      factory->BookMethod( TMVA::Types::kBDT, "BDTG", 
                           "!H:!V:NTrees=1000:BoostType=Grad:Shrinkage=0.30:UseBaggedGrad:GradBaggingFraction=0.6:SeparationType=GiniIndex:nCuts=20:NNodesMax=5" );

   if (Use["BDT"])  // Adaptive Boost
      factory->BookMethod( TMVA::Types::kBDT, "BDT", 
                           "!H:!V:NTrees=400:nEventsMin=400:MaxDepth=3:BoostType=AdaBoost:SeparationType=GiniIndex:nCuts=20:PruneMethod=NoPruning" );
   
   if (Use["BDTB"]) // Bagging
      factory->BookMethod( TMVA::Types::kBDT, "BDTB", 
                           "!H:!V:NTrees=400:BoostType=Bagging:SeparationType=GiniIndex:nCuts=20:PruneMethod=NoPruning" );

   if (Use["BDTD"]) // Decorrelation + Adaptive Boost
      factory->BookMethod( TMVA::Types::kBDT, "BDTD", 
                           "!H:!V:NTrees=400:nEventsMin=400:MaxDepth=3:BoostType=AdaBoost:SeparationType=GiniIndex:nCuts=20:PruneMethod=NoPruning:VarTransform=Decorrelate" );
   
   if (Use["RuleFit"]) // RuleFit -- TMVA implementation of Friedman's method
      factory->BookMethod( TMVA::Types::kRuleFit, "RuleFit",
                           "H:!V:RuleFitModule=RFTMVA:Model=ModRuleLinear:MinImp=0.001:RuleMinDist=0.001:NTrees=20:fEventsMin=0.01:fEventsMax=0.5:GDTau=-1.0:GDTauPrec=0.01:GDStep=0.01:GDNSteps=10000:GDErrScale=1.02" );
   
   // Train MVAs using the set of training events
   factory->TrainAllMethods();

   // ---- Evaluate all MVAs using the set of test events
   factory->TestAllMethods();

   // ----- Evaluate and compare performance of all configured MVAs
   factory->EvaluateAllMethods();    

   // --------------------------------------------------------------
   outputFile->Close();
   // Save the output

   std::cout << "==> Wrote root file: " << outputFile->GetName() << std::endl
             << "==> TMVAClassification is done!" << std::endl
             << std::endl
             << "==> To view the results, launch the GUI: \"root -l ../macros/TMVAGui.C\"" << std::endl
             << std::endl;
   // data input tests
   TFile* output2 = TFile::Open( outfileName);
   thetest->SetTrees((TTree*) output2->Get("TrainTree"),(TTree*) output2->Get("TestTree"));
   thetest->CheckAssertions();
   output2->Close();
   std::cout <<"All tests were successfull"<<std::endl;
   // Clean up
   delete factory;
}


