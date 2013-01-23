// @(#)root/tmva $Id: DumpOptionsReference.C,v 1.8 2009-01-27 17:03:25 andreas.hoecker Exp $
/**********************************************************************************
 * Project   : TMVA - a Root-integrated toolkit for multivariate data analysis    *
 * Package   : TMVA                                                               *
 * Root Macro: DumpOptionsReference                                               *
 *                                                                                *
 * This macro instantiates calls all configurable classes and dumps               *
 * the reference for options. Also dumped (in the same output file) are the       *
 * help messages for classifier tuning.                                           *
 *                                                                                *
 * This macro works together with the script "CreateOptionRef.py" that creates    *
 * the corresponding html reference file.                                         *
 **********************************************************************************/

#include <iostream>

#include "TCut.h"
#include "TFile.h"
#include "TSystem.h"
#include "TTree.h"

#include "TMVA/Config.h"

void DumpOptionsReference()
{
   std::cout << "Start DumpOptionsReference" << std::endl
             << "======================" << std::endl
             << std::endl;

   // Create a new root output file.
   TString outfileName( "Dummy.root" );
   TFile* outputFile = TFile::Open( outfileName, "RECREATE" );

   // Create the factory object. Later you can choose the methods
   // whose performance you'd like to investigate. The factory will
   // then run the performance analysis for you.
   //
   // The first argument is the base of the name of all the
   // weightfiles in the directory weight/ 
   //
   // The second argument is the output file for the training results
   // All TMVA output can be suppressed by removing the "!" (not) in 
   // front of the "Silent" argument in the option string
   // Write reference file for options
   (TMVA::gConfig()).SetWriteOptionsReference( kTRUE );

   TMVA::Factory *factory = new TMVA::Factory( "DumpOptionsReference", outputFile, "" );

   // Define the input variables that shall be used for the MVA training
   // note that you may also use variable expressions, such as: "3*var1/var2*abs(var3)"
   // [all types of expressions that can also be parsed by TTree::Draw( "expression" )]
   factory->AddVariable( "var1", 'F' );
   factory->AddVariable( "var2", 'F' );
   factory->AddVariable( "var3", 'F' );
   factory->AddVariable( "var4", 'F' );

   TFile *input(0);
   char* TMVAPath = getenv ("TMVASYS");
   if (TMVAPath!=NULL)
      printf ("Using TMVA installed in: %s\n",TMVAPath);
   else {
      printf ("ERROR: could not read environment variable TMVASYS. Is it set properly?\n");
      return 0;
   }    
   TString fname = Form("%s/test/tmva_example.root",TMVAPath);
   if (!gSystem->AccessPathName( fname )) {
       // first we try to find tmva_example.root in the local directory
      std::cout << "--- DumpOptionsReference    : Accessing " << fname << std::endl;
      input = TFile::Open( fname );
   } 
   if (!input) {
      std::cout << "ERROR: could not open data file" << std::endl;
      exit(1);
   }
   
   TTree *signal     = (TTree*)input->Get("TreeS");
   TTree *background = (TTree*)input->Get("TreeB");
   
   // ====== register trees ====================================================
   //
   // the following method is the prefered one:
   // you can add an arbitrary number of signal or background trees
   factory->AddSignalTree    ( signal,     1 );
   factory->AddBackgroundTree( background, 1 );
   
   // This would set individual event weights (the variables defined in the 
   // expression need to exist in the original TTree)
   //    for signal    : factory->SetSignalWeightExpression("weight1*weight2");
   //    for background: factory->SetBackgroundWeightExpression("weight1*weight2");

   // Apply additional cuts on the signal and background samples (can be different)
   TCut mycuts = ""; // for example: TCut mycuts = "abs(var1)<0.5 && abs(var2-0.5)<1";
   TCut mycutb = ""; // for example: TCut mycutb = "abs(var1)<0.5";

   // tell the factory to use all remaining events in the trees after training for testing:
   factory->PrepareTrainingAndTestTree( mycuts, mycutb, "" );  

   // ---- Book MVA methods
   //
   // please lookup the various method configuration options in the corresponding cxx files, eg:
   // src/MethoCuts.cxx, etc.
   // it is possible to preset ranges in the option string in which the cut optimisation should be done:
   // "...:CutRangeMin[2]=-1:CutRangeMax[2]=1"...", where [2] is the third input variable

   // Cut optimisation
   factory->BookMethod( TMVA::Types::kCuts, "Cuts", "" );
   
   // Likelihood
   factory->BookMethod( TMVA::Types::kLikelihood, "Likelihood", "" ); 

   // PDE-RS
   factory->BookMethod( TMVA::Types::kPDERS, "PDERS", "" );

   // PDE-Foam
   factory->BookMethod( TMVA::Types::kPDEFoam, "PDEFoam", "" );
  
   // K-Nearest Neighbour classifier (KNN)
   factory->BookMethod( TMVA::Types::kKNN, "KNN", "" );

   // H-Matrix (chi2-squared) method
   factory->BookMethod( TMVA::Types::kHMatrix, "HMatrix", "" ); 
   
   // Fisher discriminant
   factory->BookMethod( TMVA::Types::kFisher, "Fisher", "" );

   // Boosted Fisher
   factory->BookMethod( TMVA::Types::kFisher, "BoostedFisher", "Boost_Num=100");

   // Linear Discriminant
   factory->BookMethod( TMVA::Types::kLD, "LD", "" );

   // Function discrimination analysis (FDA) -- test of various fitters - the recommended one is Minuit (or GA or SA)
   factory->BookMethod( TMVA::Types::kFDA, "FDA_MC", "FitMethod=MC" );
   factory->BookMethod( TMVA::Types::kFDA, "FDA_GA", "FitMethod=GA" );
   factory->BookMethod( TMVA::Types::kFDA, "FDA_SA", "FitMethod=SA" );
   factory->BookMethod( TMVA::Types::kFDA, "FDA_MT", "FitMethod=MINUIT" );
   
   // TMVA ANN: MLP (recommended ANN) -- all ANNs in TMVA are Multilayer Perceptrons
   factory->BookMethod( TMVA::Types::kMLP, "MLP", "" );

   // CF(Clermont-Ferrand)ANN
   factory->BookMethod( TMVA::Types::kCFMlpANN, "CFMlpANN", "" );
  
   // Tmlp(Root)ANN
   factory->BookMethod( TMVA::Types::kTMlpANN, "TMlpANN", "" );
  
   // Support Vector Machines using three different Kernel types (Gauss, polynomial and linear)
   factory->BookMethod( TMVA::Types::kSVM, "SVM_Gauss", "" );

   // Boosted Decision Trees (second one with decorrelation)
   factory->BookMethod( TMVA::Types::kBDT, "BDT", "" );

   // RuleFit -- TMVA implementation of Friedman's method
   factory->BookMethod( TMVA::Types::kRuleFit, "RuleFit", "" );
 
   // --------------------------------------------------------------------------------------------------

   // help messages are also dumped into files
   factory->PrintHelpMessage();
   
   // Save the output
   outputFile->Close();

   std::cout << "==> Wrote root file: " << outputFile->GetName() << std::endl;
   std::cout << "==> DumpOptionsReference is done!" << std::endl;      

   delete factory;
}
