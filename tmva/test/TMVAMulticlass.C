
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

#include "TMVAMultiClassGui.C"

#ifndef __CINT__
#include "TMVA/Tools.h"
#include "TMVA/Factory.h"
#endif

using namespace TMVA;

void TMVAMulticlass(){
   TString outfileName = "TMVAMulticlass.root";
   TFile* outputFile = TFile::Open( outfileName, "RECREATE" );
   TMVA::Factory *factory = new TMVA::Factory( "TMVAClassification", outputFile,
                                               "!V:!Silent:Color:DrawProgressBar:Transformations=I;D;P;G,D:AnalysisType=multiclass" );
   factory->AddVariable( "var1", 'F' );
   factory->AddVariable( "var2", "Variable 2", "", 'F' );
   factory->AddVariable( "var3", "Variable 3", "units", 'F' );
   factory->AddVariable( "var4", "Variable 4", "units", 'F' );
   TFile *input(0);
   TString fname = "./tmva_example_multiple_background.root";
   if (!gSystem->AccessPathName( fname )) {
      // first we try to find the file in the local directory
      std::cout << "--- TMVAMulticlass   : Accessing " << fname << std::endl;
      input = TFile::Open( fname );
   }
   else {
      cout << "Creating testdata...." << std::endl;
      gROOT->ProcessLine(".L createData.C+");
      gROOT->ProcessLine("create_MultipleBackground(2000)");
      cout << " created tmva_example_multiple_background.root for tests of the multiclass features"<<endl;
      input = TFile::Open( fname );
   }
   if (!input) {
      std::cout << "ERROR: could not open data file" << std::endl;
      exit(1);
   }
   //TTree *tree     = (TTree*)input->Get("TreeR");
   TTree *signal      = (TTree*)input->Get("TreeS");
   TTree *background0 = (TTree*)input->Get("TreeB0");
   TTree *background1 = (TTree*)input->Get("TreeB1");
   TTree *background2 = (TTree*)input->Get("TreeB2");
   
   gROOT->cd( outfileName+TString(":/") );
   factory->AddTree    (signal,"Signal");
   factory->AddTree    (background0,"bg0");
   factory->AddTree    (background1,"bg1");
   factory->AddTree    (background2,"bg2");
   
   factory->PrepareTrainingAndTestTree( "", "SplitMode=Random:NormMode=NumEvents:!V" );

   factory->BookMethod( TMVA::Types::kBDT, "BDTG", "!H:!V:NTrees=1000:BoostType=Grad:Shrinkage=0.30:UseBaggedGrad:GradBaggingFraction=0.50:SeparationType=GiniIndex:nCuts=20:NNodesMax=5");
   factory->BookMethod( TMVA::Types::kMLP, "MLP2", "!H:!V:NeuronType=tanh:NCycles=100:HiddenLayers=N+5,3:TestRate=5");
   factory->BookMethod( TMVA::Types::kFDA, "FDA_GA",
                        "H:!V:Formula=(0)+(1)*x0+(2)*x1+(3)*x2+(4)*x3:ParRanges=(-1,1);(-10,10);(-10,10);(-10,10);(-10,10):FitMethod=GA:PopSize=300:Cycles=3:Steps=20:Trim=True:SaveBestGen=1" );
   
  // Train MVAs using the set of training events
   factory->TrainAllMethods();

   // ---- Evaluate all MVAs using the set of test events
   factory->TestAllMethods();

   // ----- Evaluate and compare performance of all configured MVAs
   factory->EvaluateAllMethods();

   // --------------------------------------------------------------
   
   // Save the output
   outputFile->Close();
   
   std::cout << "==> Wrote root file: " << outputFile->GetName() << std::endl;
   std::cout << "==> TMVAClassification is done!" << std::endl;
   
   delete factory;
   
   // Launch the GUI for the root macros
   if (!gROOT->IsBatch()) TMVAMultiClassGui( outfileName );
   
   
}
