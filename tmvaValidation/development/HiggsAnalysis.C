// @(#)root/tmva $Id: HiggsAnalysis.C,v 1.1 2007-03-09 13:19:38 andreas.hoecker Exp $
/**********************************************************************************
 * Project   : TMVA - a Root-integrated toolkit for multivariate data analysis    *
 * Package   : TMVA                                                               *
 * Root Macro: HiggsAnalysis                                                        *
 *                                                                                *
 * This macro gives an example on training and testing of several                 *
 * Multivariate Analyser (MVA) methods.                                           *
 *                                                                                *
 * As input file we use a toy MC sample (you find it in TMVA/examples/data).      *
 *                                                                                *
 * The methods to be used can be switched on and off by means of booleans.        *
 *                                                                                *
 * The output file "TMVA.root" can be analysed with the use of dedicated          *
 * macros (simply say: root -l <macro.C>), which can be conveniently              *
 * invoked through a GUI that will appear at the end of the run of this macro.    *
 **********************************************************************************/

#include <iostream>

#include "TCut.h"
#include "TFile.h"
#include "TSystem.h"
#include "TTree.h"

#include "TMVAGui.C"

// ---------------------------------------------------------------
// choose MVA methods to be trained + tested
Bool_t Use_Cuts            = 0;
Bool_t Use_CutsD           = 0;
Bool_t Use_CutsGA          = 1;
Bool_t Use_Likelihood      = 1;
Bool_t Use_LikelihoodD     = 0; // the "D" extension indicates decorrelated input variables (see option strings)
Bool_t Use_LikelihoodPCA   = 1; // the "PCA" extension indicates PCA-transformed input variables (see option strings)
Bool_t Use_LikelihoodKDE   = 0;
Bool_t Use_PDERS           = 1;
Bool_t Use_PDERSD          = 0;
Bool_t Use_PDERSPCA        = 0;
Bool_t Use_HMatrix         = 1;
Bool_t Use_Fisher          = 1;
Bool_t Use_MLP             = 1; // this is the recommended ANN
Bool_t Use_CFMlpANN        = 0; 
Bool_t Use_TMlpANN         = 0; 
Bool_t Use_BDT             = 1;
Bool_t Use_BDTD            = 0;
Bool_t Use_RuleFit         = 1;

// read input data file with ascii format (otherwise ROOT) ?
Bool_t ReadDataFromAsciiIFormat = kFALSE;

void HiggsAnalysis( TString myMethodList = "" ) 
{
   // explicit loading of the shared libTMVA is done in TMVAlogon.C, defined in .rootrc
   // if you use your private .rootrc, or run from a different directory, please copy the 
   // corresponding lines from .rootrc

   // methods to be processed can be given as an argument; use format:
   //
   // mylinux~> root -l HiggsAnalysis.C\(\"myMethod1\ myMethod2\ myMethod3\"\)
   //
   if (myMethodList != "") {
      Use_CutsGA = Use_CutsD = Use_Cuts
         = Use_LikelihoodKDE = Use_LikelihoodPCA = Use_LikelihoodD = Use_Likelihood
         = Use_PDERSPCA = Use_PDERSD = Use_PDERS = Use_MLP = Use_CFMlpANN = Use_TMlpANN
         = Use_HMatrix = Use_Fisher = Use_BDTD = Use_BDT = Use_RuleFit = 0;

      TList* mlist = TMVA::Tools::ParseFormatLine( myMethodList, ' ' );

      if (mlist->FindObject( "Cuts"          ) != 0) Use_Cuts          = 1; 
      if (mlist->FindObject( "CutsD"         ) != 0) Use_CutsD         = 1; 
      if (mlist->FindObject( "CutsGA"        ) != 0) Use_CutsGA        = 1; 
      if (mlist->FindObject( "Likelihood"    ) != 0) Use_Likelihood    = 1; 
      if (mlist->FindObject( "LikelihoodD"   ) != 0) Use_LikelihoodD   = 1; 
      if (mlist->FindObject( "LikelihoodPCA" ) != 0) Use_LikelihoodPCA = 1; 
      if (mlist->FindObject( "LikelihoodKDE" ) != 0) Use_LikelihoodKDE = 1; 
      if (mlist->FindObject( "PDERSPCA"      ) != 0) Use_PDERSPCA      = 1; 
      if (mlist->FindObject( "PDERSD"        ) != 0) Use_PDERSD        = 1; 
      if (mlist->FindObject( "PDERS"         ) != 0) Use_PDERS         = 1; 
      if (mlist->FindObject( "HMatrix"       ) != 0) Use_HMatrix       = 1; 
      if (mlist->FindObject( "Fisher"        ) != 0) Use_Fisher        = 1; 
      if (mlist->FindObject( "MLP"           ) != 0) Use_MLP           = 1; 
      if (mlist->FindObject( "CFMlpANN"      ) != 0) Use_CFMlpANN      = 1; 
      if (mlist->FindObject( "TMlpANN"       ) != 0) Use_TMlpANN       = 1; 
      if (mlist->FindObject( "BDTD"          ) != 0) Use_BDTD          = 1; 
      if (mlist->FindObject( "BDT"           ) != 0) Use_BDT           = 1; 
      if (mlist->FindObject( "RuleFit"       ) != 0) Use_RuleFit       = 1; 

      delete mlist;
   }
  
   std::cout << "Start Test HiggsAnalysis" << std::endl
        << "======================" << std::endl
        << std::endl;
   std::cout << "Testing all standard methods may take about 4 minutes..." << std::endl;

   // Create a new root output file.
   TFile* outputFile = TFile::Open( "TMVA.root", "RECREATE" );

   // Create the factory object. Later you can choose the methods
   // whose performance you'd like to investigate. The factory will
   // then run the performance analysis for you.
   //
   // The first argument is the base of the name of all the
   // weightfiles in the directory weight/ 
   //
   // The second argument is the output file for the training results
   TMVA::Factory *factory = new TMVA::Factory( "MVAnalysis", outputFile, "!V" );

   // if you wish to modify default settings:
   // TMVA::gConfig().variablePlotting.timesRMS = 16.0;

   // load the signal and background event samples from ROOT trees
   TFile *inputS = TFile::Open("data_higgs/thetreeS_3.root");
   TFile *inputB = TFile::Open("data_higgs/thetreeB_3.root");

   TTree *signal     = (TTree*)inputS->Get("MyTree");
   TTree *background = (TTree*)inputB->Get("MyTree");

   // global event weights (see below for setting event-wise weights)
   Double_t signalWeight     = 1.0;
   Double_t backgroundWeight = 1.0;
   
   if (!factory->SetInputTrees( signal, background, signalWeight, backgroundWeight)) exit(1);
   
   // Define the input variables that shall be used for the MVA training
   // note that you may also use variable expressions, such as: "3*var1/var2*abs(var3)"
   // [all types of expressions that can also be parsed by TTree::Draw( "expression" )]
   factory->AddVariable("mgg", 'F');
   factory->AddVariable("cosThStar", 'F');
   factory->AddVariable("pT", 'F');

   // This would set individual event weights (the variables defined in the 
   // expression need to exist in the original TTree)
   // factory->SetWeightExpression("weight1*weight2");
   
   // Apply additional cuts on the signal and background sample. 
   TCut mycut = ""; // for example: TCut mycut = "abs(var1)<0.5 && abs(var2-0.5)<1";

   // tell the factory to use all remaining events in the trees after training for testing:
   factory->PrepareTrainingAndTestTree( mycut, "NSigTrain=150000:NBkgTrain=150000:SplitMode=Random:!V" );  

   // If no numbers of events are given, half of the events in the tree are used for training, and 
   // the other haof for testing:
   //   factory->PrepareTrainingAndTestTree( mycut, "SplitMode=random:!V" );  
   // To also specify the number of testing events, use:
   //   factory->PrepareTrainingAndTestTree( mycut, 
   //                                        "NSigTrain=3000:NBkgTrain=3000:NSigTest=3000:NBkgTest=3000:SplitMode=Random:!V" );  
   // an equivalent of writing this is:
   // the old-style call:
   //   factory->PrepareTrainingAndTestTree( mycut, Ntrain, Ntest );
   // is kept for backward compatibility, but depreciated

   // ---- book MVA methods
   //
   // please lookup the various method configuration options in the corresponding cxx files, eg:
   // src/MethoCuts.cxx, etc.

   // Cut optimisation
   if (Use_Cuts) 
     factory->BookMethod( TMVA::Types::kCuts, "Cuts", "!V:MC:EffSel:MC_NRandCuts=100000:AllFSmart" );

   if (Use_CutsD) 
     factory->BookMethod( TMVA::Types::kCuts, "CutsD", "!V:MC:EffSel:MC_NRandCuts=200000:AllFSmart:VarTransform=Decorrelate" );

   if (Use_CutsGA) 
   // alternatively, use the powerful cut optimisation with a Genetic Algorithm
     factory->BookMethod( TMVA::Types::kCuts, "CutsGA",
                         "!V:GA:EffSel:GA_nsteps=40:GA_cycles=3:GA_popSize=300:GA_SC_steps=10:GA_SC_rate=5:GA_SC_factor=0.95" );

   // Likelihood
   if (Use_Likelihood) 
      factory->BookMethod( TMVA::Types::kLikelihood, "Likelihood", "!V:!TransformOutput:Spline=2:NSmoothSig[0]=0:NSmoothSig[1]=20:NSmoothSig[2]=0:NSmoothBkg[0]=40:NSmoothBkg[1]=20:NSmoothBkg[2]=2:NAvEvtPerBin=4000" ); 

   // test the decorrelated likelihood
   if (Use_LikelihoodD) 
      factory->BookMethod( TMVA::Types::kLikelihood, "LikelihoodD", "!V:!TransformOutput:Spline=3:NSmooth=5:NAvEvtPerBin=50:VarTransform=Decorrelate"); 

   if (Use_LikelihoodPCA) 
      factory->BookMethod( TMVA::Types::kLikelihood, "LikelihoodPCA", "!V:!TransformOutput:Spline=2:NSmooth=5:NAvEvtPerBin=50:VarTransform=PCA"); 
 
   // test the new kernel density estimator
   if (Use_LikelihoodKDE) 
      factory->BookMethod( TMVA::Types::kLikelihood, "LikelihoodKDE", "!V:!TransformOutput:UseKDE:KDEtype=Gauss:KDEiter=Adaptive:KDEborder=Mirror:NAvEvtPerBin=500" ); 

   // Fisher:
   if (Use_Fisher)
      factory->BookMethod( TMVA::Types::kFisher, "Fisher", "!V:Fisher:CreateMVAPdfs:NbinsMVAPdf=50:NsmoothMVAPdf=1" );    

   // the new TMVA ANN: MLP (recommended ANN)
   if (Use_MLP)
      factory->BookMethod( TMVA::Types::kMLP, "MLP", "!V:NCycles=200:HiddenLayers=N+1,N:TestRate=5" );

   // CF(Clermont-Ferrand)ANN
   if (Use_CFMlpANN)
      factory->BookMethod( TMVA::Types::kCFMlpANN, "CFMlpANN", "!V:H:NCycles=500:HiddenLayers=N,N"  ); // n_cycles:#nodes:#nodes:...  
  
   // Tmlp(Root)ANN
   if (Use_TMlpANN)
      factory->BookMethod( TMVA::Types::kTMlpANN, "TMlpANN", "!V:NCycles=200:HiddenLayers=N+1,N"  ); // n_cycles:#nodes:#nodes:...
  
   // HMatrix
   if (Use_HMatrix)
      factory->BookMethod( TMVA::Types::kHMatrix, "HMatrix", "!V" ); // H-Matrix (chi2-squared) method
  
   // PDE - RS method
   if (Use_PDERS)
      factory->BookMethod( TMVA::Types::kPDERS, "PDERS", 
                           "!V:VolumeRangeMode=Adaptive:KernelEstimator=Gauss:GaussSigma=0.3:NEventsMin=400:NEventsMax=600:InitialScale=0.99" );
   
   if (Use_PDERSD) 
      factory->BookMethod( TMVA::Types::kPDERS, "PDERSD", 
                           "!V:VolumeRangeMode=Adaptive:KernelEstimator=Gauss:GaussSigma=0.3:NEventsMin=400:NEventsMax=600:InitialScale=0.99:VarTransform=Decorrelate" );

   if (Use_PDERSPCA) 
      factory->BookMethod( TMVA::Types::kPDERS, "PDERSPCA", 
                           "!V:VolumeRangeMode=Adaptive:KernelEstimator=Gauss:GaussSigma=0.3:NEventsMin=400:NEventsMax=600:InitialScale=0.99:VarTransform=PCA" );
  
   // Boosted Decision Trees
   if (Use_BDT)
      factory->BookMethod( TMVA::Types::kBDT, "BDT", 
                           "!V:NTrees=400:BoostType=AdaBoost:SeparationType=GiniIndex:nEventsMin=20:nCuts=20:PruneMethod=CostComplexity:PruneStrength=3.5");
   if (Use_BDTD)
      factory->BookMethod( TMVA::Types::kBDT, "BDTD", 
                           "!V:NTrees=400:BoostType=AdaBoost:SeparationType=GiniIndex:nEventsMin=20:nCuts=20:PruneMethod=CostComplexity:PruneStrength=3.5:VarTransform=Decorrelate");

   // Friedman's RuleFit method
   if (Use_RuleFit)
      factory->BookMethod( TMVA::Types::kRuleFit, "RuleFit",
                           "!V:NTrees=20:SampleFraction=-1:nEventsMin=60:nCuts=20:SeparationType=GiniIndex:Model=ModRuleLinear:GDTau=0.6:GDTauMin=0.0:GDTauMax=1.0:GDNTau=20:GDStep=0.01:GDNSteps=5000:GDErrScale=1.1:RuleMinDist=0.0001:MinImp=0.001" );


   // ---- Now you can tell the factory to train, test, and evaluate the MVAs. 

   // Train MVAs using the set of training events
   factory->TrainAllMethods();

   // Evaluate all MVAs using the set of test events
   factory->TestAllMethods();

   // Evaluate and compare performance of all configured MVAs
   factory->EvaluateAllMethods();    
  
   // Save the output.
   outputFile->Close();

   std::cout << "==> wrote root file TMVA.root" << std::endl;
   std::cout << "==> HiggsAnalysis is done!" << std::endl;      

   // clean up
   delete factory;

   // open the GUI for the root macros
   TMVAGui();
} 
