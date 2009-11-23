/**********************************************************************************
 * Project   : TMVA - a Root-integrated toolkit for multivariate data analysis    *
 * Package   : TMVA                                                               *
 * Exectuable: TMVAClassificationApplication                                      *
 *                                                                                *
 * This macro provides a simple example on how to use the trained classifiers     *
 * within an analysis module                                                      *
 **********************************************************************************/

#include <cstdlib>
#include <vector>
#include <iostream>
#include <iomanip>
#include <map>
#include <string>

#include "TFile.h"
#include "TTree.h"
#include "TString.h"
#include "TSystem.h"
#include "TROOT.h"
#include "TPluginManager.h"
#include "TStopwatch.h"

#include "TMVA/Reader.h"
#include "TMVA/Tools.h"
#include "TMVA/MethodCuts.h"
#include "TMVA/MethodMLP.h"

int main( int argc, char** argv ) 
{
   //---------------------------------------------------------------
   // default MVA methods to be trained + tested
   std::map<TString,int> Use;
   Use["BDTD"         ] = 1;  // WORKS   -  JS
   Use["BDT"          ] = 1;  // WORKS   -  JS
   Use["CFMlpANN"     ] = 1;  // WORKS   -  JS
   Use["CutsGA"       ] = 1;  // WORKS   -  JS
   Use["CutsSA"       ] = 1;  // WORKS   -  JS
   Use["CutsD"        ] = 1;  // WORKS   -  JS
   Use["Cuts"         ] = 1;  // WORKS   -  JS
   Use["FDA_MT"       ] = 1;  // WORKS   -  JS
   Use["Fisher"       ] = 1;  // WORKS   -  JS
   Use["HMatrix"      ] = 1;  // WORKS   -  JS
   Use["KNN"          ] = 1;  // WORKS   -  JS
   Use["LikelihoodD"  ] = 1;  // WORKS   -  JS
   Use["LikelihoodGSD"] = 1;  // WORKS   -  JS (small deviations due to PDF evaluation)
   Use["LikelihoodKDE"] = 1;  // WORKS   -  JS
   Use["LikelihoodMIX"] = 1;  // WORKS   -  JS
   Use["LikelihoodPCA"] = 1;  // WORKS   -  JS
   Use["Likelihood"   ] = 1;  // WORKS   -  JS
   Use["MLP"          ] = 1;  // WORKS   -  JS
   Use["PDERSkNN"     ] = 0;  // PROBLEM - old: two BST, new: one BST
   Use["PDERS"        ] = 0;  // PROBLEM
   Use["RuleFit"      ] = 1;  // WORKS   -  JS
   Use["SVM_Gauss"    ] = 1;  // WORKS   -  JS
   Use["SVM_Lin"      ] = 1;  // WORKS   -  JS
   Use["SVM_Poly"     ] = 1;  // WORKS   -  JS
   Use["TMlpANN"      ] = 1;  // WORKS   -  JS
   // ---------------------------------------------------------------

   std::cout << std::endl;
   std::cout << "==> Start TMVAClassificationApplication397" << std::endl;

   std::cout << "Running the following methods" << std::endl;
   if (argc>1) {
      for (std::map<TString,int>::iterator it = Use.begin(); it != Use.end(); it++) it->second = 0;
   }
   for (int i=1; i<argc; i++) {
      std::string regMethod(argv[i]);
      if (Use.find(regMethod) == Use.end()) {
         std::cout << "Method " << regMethod << " not known in TMVA under this name. Please try one of:" << std::endl;
         for (std::map<TString,int>::iterator it = Use.begin(); it != Use.end(); it++) std::cout << it->first << " ";
         std::cout << std::endl;
         return 1;
      }
      Use[regMethod] = kTRUE;
   }

   //
   // create the Reader object
   //
   TMVA::Reader *reader = new TMVA::Reader( "!Color:!Silent" );    

   // create a set of variables and declare them to the reader
   // - the variable names must corresponds in name and type to 
   // those given in the weight file(s) that you use
   Float_t myvar1, myvar2;
   Float_t var3, var4;
   reader->AddVariable( "myvar1 := var1+var2", &myvar1 );
   reader->AddVariable( "myvar2 := var1-var2", &myvar2 );
   reader->AddVariable( "var3",                &var3 );
   reader->AddVariable( "var4",                &var4 );



   //
   // book the MVA methods
   //
   TString dir    = "../development/weights397/";
   TString prefix = "TMVAnalysis";

   // book method(s)
   for (std::map<TString,int>::iterator it = Use.begin(); it != Use.end(); it++) {
      if (!it->second) continue;
      TString methodName = it->first + " method";
      TString weightfile = dir + prefix + "_" + it->first + ".weights.txt";
      reader->BookMVA( methodName, weightfile );
   }
   
   // book output histograms
   UInt_t nbin = 100;
   std::map<TString,TH1F*> hist;
   TString ms = "";
   ms = "BDTD";          if (Use[ms]) hist[ms] = new TH1F( "MVA_"+ms, "MVA_"+ms, nbin,  -0.8, 0.8 );
   ms = "BDT";           if (Use[ms]) hist[ms] = new TH1F( "MVA_"+ms, "MVA_"+ms, nbin,  -0.8, 0.8 );
   ms = "CFMlpANN";      if (Use[ms]) hist[ms] = new TH1F( "MVA_"+ms, "MVA_"+ms, nbin,     0, 1.0 );
   ms = "FDA_MT";        if (Use[ms]) hist[ms] = new TH1F( "MVA_"+ms, "MVA_"+ms, nbin,  -2.0, 3.0 );
   ms = "Fisher";        if (Use[ms]) hist[ms] = new TH1F( "MVA_"+ms, "MVA_"+ms, nbin,    -4, 4.0 );
   ms = "HMatrix";       if (Use[ms]) hist[ms] = new TH1F( "MVA_"+ms, "MVA_"+ms, nbin, -0.95, 1.55 );
   ms = "KNN";           if (Use[ms]) hist[ms] = new TH1F( "MVA_"+ms, "MVA_"+ms, nbin,     0, 1.0 );
   ms = "LikelihoodD";   if (Use[ms]) hist[ms] = new TH1F( "MVA_"+ms, "MVA_"+ms, nbin,     0, 1.0 );
   ms = "LikelihoodGSD"; if (Use[ms]) hist[ms] = new TH1F( "MVA_"+ms, "MVA_"+ms, nbin,     0, 1.0 );
   ms = "LikelihoodKDE"; if (Use[ms]) hist[ms] = new TH1F( "MVA_"+ms, "MVA_"+ms, nbin,     0, 1.0 );
   ms = "LikelihoodMIX"; if (Use[ms]) hist[ms] = new TH1F( "MVA_"+ms, "MVA_"+ms, nbin,     0, 1.0 );
   ms = "LikelihoodPCA"; if (Use[ms]) hist[ms] = new TH1F( "MVA_"+ms, "MVA_"+ms, nbin,     0, 1.0 );
   ms = "Likelihood";    if (Use[ms]) hist[ms] = new TH1F( "MVA_"+ms, "MVA_"+ms, nbin,     0, 1.0 );
   ms = "MLP";           if (Use[ms]) hist[ms] = new TH1F( "MVA_"+ms, "MVA_"+ms, nbin, -1.25, 1.5 );
   ms = "PDERSkNN";      if (Use[ms]) hist[ms] = new TH1F( "MVA_"+ms, "MVA_"+ms, nbin,     0, 1.0 );
   ms = "PDERS";         if (Use[ms]) hist[ms] = new TH1F( "MVA_"+ms, "MVA_"+ms, nbin,     0, 1.0 );
   ms = "RuleFit";       if (Use[ms]) hist[ms] = new TH1F( "MVA_"+ms, "MVA_"+ms, nbin,  -2.0, 2.0 );
   ms = "SVM_Gauss";     if (Use[ms]) hist[ms] = new TH1F( "MVA_"+ms, "MVA_"+ms, nbin,   0.0, 1.0 );
   ms = "SVM_Lin";       if (Use[ms]) hist[ms] = new TH1F( "MVA_"+ms, "MVA_"+ms, nbin,   0.0, 1.0 );
   ms = "SVM_Poly";      if (Use[ms]) hist[ms] = new TH1F( "MVA_"+ms, "MVA_"+ms, nbin,   0.0, 1.0 );
   ms = "TMlpANN";       if (Use[ms]) hist[ms] = new TH1F( "MVA_"+ms, "MVA_"+ms, nbin,   0.0, 1.0 );

   // Prepare input tree (this must be replaced by your data source)
   // in this example, there is a toy tree with signal and one with background events
   // we'll later on use only the "signal" events for the test in this example.
   //   
   TString fname = "../development/weights397/TMVApp.root";   
   std::cout << "--- Accessing data file: " << fname << std::endl;
   TFile *input = TFile::Open( fname );
   if (!input) {
      std::cout << "ERROR: could not open data file: " << fname << std::endl;
      exit(1);
   }

   //
   // prepare the tree
   // - here the variable names have to corresponds to your tree
   // - you can use the same variables as above which is slightly faster,
   //   but of course you can use different ones and copy the values inside the event loop
   //
   TTree* theTree = (TTree*)input->Get("Results");
   std::cout << "--- Select signal sample" << std::endl;
   Float_t userVar1, userVar2;
   theTree->SetBranchAddress( "var1", &userVar1 );
   theTree->SetBranchAddress( "var2", &userVar2 );
   theTree->SetBranchAddress( "var3", &var3 );
   theTree->SetBranchAddress( "var4", &var4 );
   std::map<TString,Float_t> oldRes;
   for (std::map<TString,int>::iterator it = Use.begin(); it != Use.end(); it++) {
      if (!it->second) continue;
      theTree->SetBranchAddress( it->first, &oldRes[it->first] );
   }



   // efficiency calculator for cut method
   std::map<TString,Int_t> nSelCuts;
   nSelCuts["Cuts"  ] = 0;
   nSelCuts["CutsD" ] = 0;
   nSelCuts["CutsGA"] = 0;
   nSelCuts["CutsSA"] = 0;
   Double_t effS = 0.7;

   std::cout << "--- Processing: " << theTree->GetEntries() << " events" << std::endl;
   TStopwatch sw;
   sw.Start();
   for (Long64_t ievt=0; ievt<theTree->GetEntries();ievt++) {
      //for (Long64_t ievt=0; ievt<2;ievt++) {

      if (ievt%1000 == 0){
         std::cout << "--- ... Processing event: " << ievt << std::endl;
      }

      theTree->GetEntry(ievt);

      myvar1 = userVar1 + userVar2;
      myvar2 = userVar1 - userVar2;

      if(ievt<2) {
         std::cout << std::endl << "event " << ievt << ": " << myvar1 << ", " << myvar2 << ", " << var3 << ", " << var4 << std::endl;
      }

      // 
      // return the MVAs and fill to histograms
      // 

      if (ievt == 0) 
         std::cout << std::setw(15) << std::left << "Method" << "  : " 
                   << std::setw(10) << "Rel 3.9.7" << " | " 
                   << std::setw(10) << "New rel" << " | " << "Difference (if > 1e-06)"
                   << std::endl;
      
      // Cuts is a special case: give the desired signal efficienciy
      for (std::map<TString,int>::iterator it = Use.begin(); it != Use.end(); it++) {
         if (! it->second) continue;
         const TString& ms = it->first;
         if(ms.BeginsWith("Cuts")) {
            Float_t val = reader->EvaluateMVA( ms + " method", effS );
            if (val>.5) nSelCuts[ms]++;
            Float_t diff = val-oldRes[it->first];
            if(ievt<2) {
               std::cout << std::setw(15) << std::left << ms << "  : " 
                         << std::setw(10) << oldRes[ms] << " | " 
                         << std::setw(10) << val << " | " << diff
                         << std::endl;
            }
         } else {
            Float_t val = reader->EvaluateMVA( ms+" method" );
            Float_t diff = val-oldRes[it->first];
            if(TMath::Abs(diff)<1e-06) diff=0;
            if(ievt<2) {
               std::cout << std::setw(15) << std::left << ms << "  : " 
                         << std::setw(10) << oldRes[ms] << " | " 
                         << std::setw(10) << val << " | " << diff
                         << std::endl;
            }
            hist[ms]->Fill(val);
         }
      }
   }

   // get elapsed time
   sw.Stop();
   std::cout << "--- End of event loop: "; sw.Print();

   // get efficiency for cuts classifier
   for (std::map<TString,int>::iterator it = Use.begin(); it != Use.end(); it++) {
      const TString& ms = it->first;
      if (!it->second || !ms.BeginsWith("Cuts")) continue;
      if (ms) std::cout << "--- Efficiency for " << ms << " method: "
                        << double(nSelCuts[ms])/theTree->GetEntries()
                        << " (for a required signal efficiency of " << effS << ")" << std::endl;
   }

   //
   // write histograms
   //
   TFile *target  = new TFile( "TMVApp.root","RECREATE" );
   std::map<TString,TH1F*>::iterator hIt = hist.begin();
   for(;hIt!=hist.end(); ++hIt) hIt->second->Write();
   target->Close();

   std::cout << "--- Created root file: \"TMVApp.root\" containing the MVA output histograms" << std::endl;
  
   delete reader;
    
   std::cout << "==> TMVAClassificationApplication397 is done!" << std::endl << std::endl;
}
