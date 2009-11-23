#include <iostream>

#include "TCut.h"
#include "TFile.h"
#include "TSystem.h"
#include "TTree.h"

void simpletest( ) 
{   
   TString outfileName( "TMVA.root" );
   TFile* outputFile = TFile::Open( outfileName, "RECREATE" );

   TMVA::Factory *factory = new TMVA::Factory( "TMVAnalysis", outputFile, Form("!V:%sColor", gROOT->IsBatch()?"!":"") );

   TTree *sigtree, *bgtree;
   Float_t v1sig,v2sig,v1bg,v2bg;
   sigtree = new TTree("sigtree","Example signal tree for PDE-RS tests");
   bgtree = new TTree("bgtree","Example BG tree for PDE-RS tests");
   sigtree->Branch("var1",&v1sig,"var1/F");
   sigtree->Branch("var2",&v2sig,"var2/F");
   bgtree->Branch("var1",&v1bg,"var1/F");
   bgtree->Branch("var2",&v2bg,"var2/F");

   // First example from hep-ex/0211019v1: two bivariate Gaussian distributions with shifted means
   Float_t meanx_sig=4, meany_sig=3.5, meanx_bg=3, meany_bg=4.5;
   Float_t sigmax_sig=0.75, sigmay_sig=0.75, sigmax_bg=1, sigmay_bg=1;
   TRandom3 rnd;
   for (Int_t i=0; i<400000; i++) {
     v1sig=rnd.Gaus(meanx_sig, sigmax_sig);
     v2sig=rnd.Gaus(meany_sig, sigmay_sig);
     sigtree->Fill();
     v1bg=rnd.Gaus(meanx_bg, sigmax_bg);
     v2bg=rnd.Gaus(meany_bg, sigmay_bg);
     bgtree->Fill();
   }

   Float_t signalWeight     = 1.0;
   Float_t backgroundWeight = 1.0;
   
   factory->AddSignalTree    ( sigtree,     signalWeight );
   factory->AddBackgroundTree( bgtree, backgroundWeight );
   
   factory->AddVariable("var1", 'F');
   factory->AddVariable("var2", 'F');

   TCut mycut = "";
   factory->PrepareTrainingAndTestTree( mycut ,"NsigTrain=0:NbkgTrain=0" );  
   //   factory->BookMethod( TMVA::Types::kLikelihood, "Likelihood", "Spline=2:NSmooth=3"   ); 
   factory->BookMethod( TMVA::Types::kFisher, "Fisher", "Fisher"   ); 
   //   factory->BookMethod( TMVA::Types::kPDERS, "PDERS","V:VolumeRangeMode=Unscaled:DeltaFrac=0.18:NEventsMin=0.01" );


   cout << "Training..." << endl;
   //   factory->TrainAllMethods();
   cout << "Testing..." << endl;
   factory->TestAllMethods();
   cout << "Evaluating..." << endl;
   factory->EvaluateAllMethods();    

   // --------------------------------------------------------------
   
   // Save the output
   outputFile->Close();

   // Clean up
   delete factory;

}
