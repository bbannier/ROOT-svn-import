// Authors: Christoph Rosemann, Eckhard von Toerne   July 2010
// TMVA unit tests

#include "Riostream.h"
#include "TSystem.h"
#include "TROOT.h"
#include "TPluginManager.h"
#include "TBenchmark.h"
#include "TApplication.h"
#include "tmvaut/UnitTestSuite.h"

#include "tmvaut/utEvent.h"
#include "tmvaut/utVariableInfo.h"
#include "tmvaut/utDataSetInfo.h"
#include "tmvaut/utDataSet.h"
#include "tmvaut/utFactory.h"
#include "tmvaut/utWeightHandling.h"
#include "tmvaut/utReader.h"
#include "tmvaut/MethodUnitTestWithROCLimits.h"
#include "tmvaut/MethodUnitTestWithComplexData.h"
#include "tmvaut/utSphereData.h"
#include "tmvaut/RegressionUnitTestWithDeviation.h"
#include "tmvaut/ReaderTest.h"
#include "TMVA/Types.h"

using namespace UnitTesting;
using namespace std;

void addClassificationTests( UnitTestSuite& TMVA_test, bool full=true)
{

   
   // problem with the make function of the Gauss transformation when only some variables are selected 
   TMVA_test.addTest(new MethodUnitTestWithROCLimits( TMVA::Types::kLD, "LDN_selVar_Gauss", "H:!V:VarTransform=G(var4,myvar1)" , 0.90, 0.95) ); 
   TMVA_test.addTest(new MethodUnitTestWithROCLimits( TMVA::Types::kLD, "LDN_selVar_P", "H:!V:VarTransform=P(var3,var4)" , 0.90, 0.95) ); 
   TMVA_test.addTest(new MethodUnitTestWithROCLimits( TMVA::Types::kLD, "LDN_selVar_D", "H:!V:VarTransform=D(myvar1,myvar2,var3)" , 0.90, 0.95) ); 
   TMVA_test.addTest(new MethodUnitTestWithROCLimits( TMVA::Types::kLD, "LDN_selVar_N", "H:!V:VarTransform=N(myvar1,myvar2,var3)" , 0.90, 0.95) ); 
   TMVA_test.addTest(new MethodUnitTestWithROCLimits( TMVA::Types::kLD, "LDN_selVar_Uniform", "H:!V:VarTransform=U(var4,myvar1)" , 0.89, 0.93) ); 


   TMVA_test.addTest(new MethodUnitTestWithROCLimits( TMVA::Types::kCuts, "CutsGA",
                                                      "H:!V:FitMethod=GA:CutRangeMin[0]=-10:CutRangeMax[0]=10:VarProp[1]=FMax:EffSel:Steps=20:Cycles=4:PopSize=300:SC_steps=10:SC_rate=5:SC_factor=0.95" , 0.4, 0.5) );

   if (full){
      TMVA_test.addTest(new MethodUnitTestWithROCLimits( TMVA::Types::kCuts, "Cuts",
                                                         "!H:!V:FitMethod=MC:EffSel:SampleSize=20000:VarProp=FSmart", 0.4, 0.5) );
      TMVA_test.addTest(new MethodUnitTestWithROCLimits( TMVA::Types::kCuts, "CutsD",
                                                         "!H:!V:FitMethod=MC:EffSel:SampleSize=20000:VarProp=FSmart:VarTransform=Decorrelate" , 0.65, 0.75) );
      TMVA_test.addTest(new MethodUnitTestWithROCLimits( TMVA::Types::kCuts, "CutsPCA", "!H:!V:FitMethod=MC:EffSel:SampleSize=20000:VarProp=FSmart:VarTransform=PCA" , 0.5, 0.6) );
      TMVA_test.addTest(new MethodUnitTestWithROCLimits( TMVA::Types::kCuts, "CutsSA",
                                                         "!H:!V:FitMethod=SA:EffSel:MaxCalls=15000:KernelTemp=IncAdaptive:InitialTemp=1e+6:MinTemp=1e-6:Eps=1e-10:UseDefaultScale" , 0.4, 0.5) );
      TMVA_test.addTest(new MethodUnitTestWithROCLimits( TMVA::Types::kLikelihood, "Likelihood",
                                                         "H:!V:TransformOutput:PDFInterpol=Spline2:NSmoothSig[0]=20:NSmoothBkg[0]=20:NSmoothBkg[1]=10:NSmooth=1:NAvEvtPerBin=50" , 0.7, 0.8) );
   }
   TMVA_test.addTest(new MethodUnitTestWithROCLimits( TMVA::Types::kLikelihood, "LikelihoodD",
                                                      "!H:!V:!TransformOutput:PDFInterpol=Spline2:NSmoothSig[0]=20:NSmoothBkg[0]=20:NSmooth=5:NAvEvtPerBin=50:VarTransform=Decorrelate" , 0.90, 0.93) );
   if (full){
      TMVA_test.addTest(new MethodUnitTestWithROCLimits( TMVA::Types::kLikelihood, "LikelihoodPCA",
                                                         "!H:!V:!TransformOutput:PDFInterpol=Spline2:NSmoothSig[0]=20:NSmoothBkg[0]=20:NSmooth=5:NAvEvtPerBin=50:VarTransform=PCA" , 0.90, 0.93) );
      TMVA_test.addTest(new MethodUnitTestWithROCLimits( TMVA::Types::kLikelihood, "LikelihoodMIX",
                                                         "!H:!V:!TransformOutput:PDFInterpolSig[0]=KDE:PDFInterpolBkg[0]=KDE:PDFInterpolSig[1]=KDE:PDFInterpolBkg[1]=KDE:PDFInterpolSig[2]=Spline2:PDFInterpolBkg[2]=Spline2:PDFInterpolSig[3]=Spline2:PDFInterpolBkg[3]=Spline2:KDEtype=Gauss:KDEiter=Nonadaptive:KDEborder=None:NAvEvtPerBin=50" , 0.7, 0.8) );
   }
   TMVA_test.addTest(new MethodUnitTestWithROCLimits( TMVA::Types::kPDERS, "PDERS",
                                                      "!H:!V:NormTree=T:VolumeRangeMode=Adaptive:KernelEstimator=Gauss:GaussSigma=0.1:NEventsMin=100:NEventsMax=600" , 0.8, 0.85) );
   if (full) TMVA_test.addTest(new MethodUnitTestWithROCLimits( TMVA::Types::kPDERS, "PDERSkNN",
                                                                "!H:!V:VolumeRangeMode=kNN:KernelEstimator=Gauss:GaussSigma=0.3:NEventsMin=400:NEventsMax=600" , 0.8, 0.85) );
   if (full) TMVA_test.addTest(new MethodUnitTestWithROCLimits( TMVA::Types::kPDERS, "PDERSD",
                                                                "!H:!V:VolumeRangeMode=Adaptive:KernelEstimator=Gauss:GaussSigma=0.3:NEventsMin=100:NEventsMax=600:VarTransform=Decorrelate" , 0.90, 0.93) );
   if (full) TMVA_test.addTest(new MethodUnitTestWithROCLimits( TMVA::Types::kPDERS, "PDERSPCA",
                                                                "!H:!V:VolumeRangeMode=Adaptive:KernelEstimator=Gauss:GaussSigma=0.3:NEventsMin=100:NEventsMax=600:VarTransform=PCA" , 0.90, 0.93) );
   TMVA_test.addTest(new MethodUnitTestWithROCLimits( TMVA::Types::kPDEFoam, "PDEFoam",
                                                      "!H:!V:SigBgSeparate=F:TailCut=0.001:VolFrac=0.0666:nActiveCells=500:nSampl=2000:nBin=5:Nmin=100:Kernel=None:Compress=T" , 0.79, 0.85) );
   TMVA_test.addTest(new MethodUnitTestWithROCLimits( TMVA::Types::kPDEFoam, "PDEFoam_SigBgSeparate",
                                                      "!H:!V:SigBgSeparate=T:TailCut=0.001:VolFrac=0.0666:nActiveCells=500:nSampl=2000:nBin=5:Nmin=10:Kernel=None:Compress=T" , 0.75, 0.85) );
   TMVA_test.addTest(new MethodUnitTestWithROCLimits( TMVA::Types::kKNN, "KNN",
                                                      "H:nkNN=20:ScaleFrac=0.8:SigmaFact=1.0:Kernel=Gaus:UseKernel=F:UseWeight=T:!Trim" , 0.8, 0.9) );
   //if (full) TMVA_test.addTest(new MethodUnitTestWithROCLimits( TMVA::Types::kHMatrix, "HMatrix", "!H:!V:VarTransform=G" , 0.88, 0.98) );
   TMVA_test.addTest(new MethodUnitTestWithROCLimits( TMVA::Types::kFisher, "Fisher",
                                                      "H:!V:Fisher:CreateMVAPdfs:PDFInterpolMVAPdf=Spline2:NbinsMVAPdf=60:NsmoothMVAPdf=10" , 0.90, 0.94) );
   if (full) TMVA_test.addTest(new MethodUnitTestWithROCLimits( TMVA::Types::kFisher, "FisherG", "H:!V:VarTransform=Gauss" , 0.90, 0.94) );
   TMVA_test.addTest(new MethodUnitTestWithROCLimits( TMVA::Types::kFisher, "BoostedFisher",
                                                      "H:!V:Boost_Num=10:Boost_Transform=log:Boost_Type=AdaBoost:Boost_AdaBoostBeta=0.2", 0.90, 0.94) );
   TMVA_test.addTest(new MethodUnitTestWithROCLimits(TMVA::Types::kLD, "LD","!H:!V", 0.90, 0.94) );
   if (full) TMVA_test.addTest(new MethodUnitTestWithROCLimits( TMVA::Types::kLD, "LD2", "H:!V:VarTransform=None" , 0.90, 0.94) );
   if (full) TMVA_test.addTest(new MethodUnitTestWithROCLimits( TMVA::Types::kLD, "LDN", "H:!V:VarTransform=N" , 0.90, 0.94) );
   if (full) TMVA_test.addTest(new MethodUnitTestWithROCLimits( TMVA::Types::kLD, "LDN_V_", "H:!V:VarTransform=N(_V_)" , 0.90, 0.94) );
   if (full) TMVA_test.addTest(new MethodUnitTestWithROCLimits( TMVA::Types::kLD, "LDN_selVar", "H:!V:VarTransform=N(myvar1,var3)" , 0.90, 0.94) );
   if (full) TMVA_test.addTest(new MethodUnitTestWithROCLimits( TMVA::Types::kLD, "LDN_selVar_Sig", "H:!V:VarTransform=N(myvar1,var3)_Signal" , 0.88, 0.90) );
   //
   // this does not work yet, starting with rev3842x this creates an error. 
   // TMVA_test.addTest(new MethodUnitTestWithROCLimits( TMVA::Types::kLD, "LDN_combSelVar", "H:!V:VarTransform=N(myvar1,var3)_Signal+P(_V_)+D(myvar2,var4)+G(var4,myvar1)_Background" , 0.88, 0.98) );

   TString baseFDAstring="!H:!V:Formula=(0)+(1)*x0+(2)*x1+(3)*x2+(4)*x3:ParRanges=(-1,1);(-10,10);(-10,10);(-10,10);(-10,10):";
   if (full) TMVA_test.addTest(new MethodUnitTestWithROCLimits( TMVA::Types::kFDA, "FDA_MC",
                                                                baseFDAstring+"FitMethod=MC:SampleSize=5000:Sigma=0.1" , 0.85, 0.92) );
   if (full) TMVA_test.addTest(new MethodUnitTestWithROCLimits( TMVA::Types::kFDA, "FDA_GA",
                                                                baseFDAstring+"FitMethod=GA:PopSize=100:Cycles=3:Steps=20:Trim=True:SaveBestGen=1" , 0.90, 0.94) );
   if (full) TMVA_test.addTest(new MethodUnitTestWithROCLimits( TMVA::Types::kFDA, "FDA_SA",
                                                                baseFDAstring+"FitMethod=SA:MaxCalls=5000:KernelTemp=IncAdaptive:InitialTemp=1e+6:MinTemp=1e-6:Eps=1e-10:UseDefaultScale" , 0.89, 0.94) );
   TMVA_test.addTest(new MethodUnitTestWithROCLimits( TMVA::Types::kMLP, "MLP", "H:!V:NeuronType=tanh:VarTransform=N:NCycles=200:HiddenLayers=N+5:TestRate=5:!UseRegulator" , 0.91, 0.93) );
   TMVA_test.addTest(new MethodUnitTestWithROCLimits( TMVA::Types::kMLP, "MLPBFGS", "H:!V:NeuronType=tanh:VarTransform=N:NCycles=200:HiddenLayers=N+5:TestRate=5:TrainingMethod=BFGS:!UseRegulator" , 0.91, 0.93) );
   if (full) TMVA_test.addTest(new MethodUnitTestWithROCLimits( TMVA::Types::kMLP, "MLPBNN", "H:!V:NeuronType=tanh:VarTransform=N:NCycles=200:HiddenLayers=N+5:TestRate=5:TrainingMethod=BFGS:UseRegulator" , 0.91, 0.93) ); // BFGS training with bayesian regulators
   if (full) TMVA_test.addTest(new MethodUnitTestWithROCLimits( TMVA::Types::kCFMlpANN, "CFMlpANN", "!H:!V:NCycles=200:HiddenLayers=N+1,N"  , 0.7, 0.94) ); // n_cycles:#nodes:#nodes:...
   if (full) TMVA_test.addTest(new MethodUnitTestWithROCLimits( TMVA::Types::kTMlpANN, "TMlpANN", "!H:!V:NCycles=200:HiddenLayers=N+1,N:LearningMethod=BFGS:ValidationFraction=0.3"  , 0.90, 0.94) ); // n_cycles:#nodes:#nodes:...
   TMVA_test.addTest(new MethodUnitTestWithROCLimits( TMVA::Types::kSVM, "SVM", "Gamma=0.25:Tol=0.001:VarTransform=Norm" , 0.90, 0.94) );
   TMVA_test.addTest(new MethodUnitTestWithROCLimits( TMVA::Types::kBDT, "BDTG",
                                                      "!H:!V:NTrees=400:BoostType=Grad:Shrinkage=0.30:UseBaggedGrad:GradBaggingFraction=0.6:SeparationType=GiniIndex:nCuts=20:NNodesMax=7" , 0.88, 0.92) );
   TMVA_test.addTest(new MethodUnitTestWithROCLimits( TMVA::Types::kBDT, "BDT",
                                                      "!H:!V:NTrees=400:nEventsMin=100:MaxDepth=3:BoostType=AdaBoost:SeparationType=GiniIndex:nCuts=10:PruneMethod=NoPruning" , 0.88, 0.92) );
   if (full) TMVA_test.addTest(new MethodUnitTestWithROCLimits( TMVA::Types::kBDT, "BDTB",
                                                                "!H:!V:NTrees=400:nEventsMin=100:BoostType=Bagging:SeparationType=GiniIndex:nCuts=20:PruneMethod=NoPruning" , 0.82, 0.92) );
   if (full) TMVA_test.addTest(new MethodUnitTestWithROCLimits( TMVA::Types::kBDT, "BDTD",
                                                                "!H:!V:NTrees=400:nEventsMin=200:MaxDepth=3:BoostType=AdaBoost:SeparationType=GiniIndex:nCuts=10:PruneMethod=NoPruning:VarTransform=Decorrelate" , 0.88, 0.98) );
   if (full) TMVA_test.addTest(new MethodUnitTestWithROCLimits( 
   TMVA::Types::kBDT, "BDTMitFisher", "!H:V:NTrees=150:NCuts=101:MaxDepth=1:UseFisherCuts:UseExclusiveVars:MinLinCorrForFisher=0.",0.88, 0.92));

   if (full) TMVA_test.addTest(new MethodUnitTestWithROCLimits( TMVA::Types::kRuleFit, "RuleFit",
                                                                "H:!V:RuleFitModule=RFTMVA:Model=ModRuleLinear:MinImp=0.001:RuleMinDist=0.001:NTrees=20:fEventsMin=0.01:fEventsMax=0.5:GDTau=-1.0:GDTauPrec=0.01:GDStep=0.01:GDNSteps=10000:GDErrScale=1.02" , 0.88, 0.92) );
   if (full) {
      TPluginManager* pluginManager = gROOT->GetPluginManager();
      pluginManager->AddHandler("TMVA@@MethodBase", "BDTPlug", "TMVA::MethodBDT", "TMVA.1", "MethodBDT(TString,TString,DataSetInfo&,TString)");
      TMVA_test.addTest(new MethodUnitTestWithROCLimits( TMVA::Types::kPlugins, "BDTPlug", "!H:!V:NTrees=400", 0.8, 0.95 ));
   }
}

void addRegressionTests( UnitTestSuite& TMVA_test, bool full=true)
{
   TMVA_test.addTest(new RegressionUnitTestWithDeviation( TMVA::Types::kLD, "LDG", "!H:!V:VarTransform=G",      15., 30.,    15., 25. ));
   TMVA_test.addTest(new RegressionUnitTestWithDeviation( TMVA::Types::kLD, "LDG1", "!H:!V:VarTransform=G(_V1_)",      15., 30.,    15., 20. ));
   TMVA_test.addTest(new RegressionUnitTestWithDeviation( TMVA::Types::kLD, "LDP2", "!H:!V:VarTransform=P(_V0_,_V1_)",      15., 25.,    13., 20. ));
   TMVA_test.addTest(new RegressionUnitTestWithDeviation( TMVA::Types::kLD, "LDU1", "!H:!V:VarTransform=G(_V1_)",      15., 30.,    15., 20. ));

   TMVA_test.addTest(new RegressionUnitTestWithDeviation( TMVA::Types::kLD, "LD", "!H:!V:VarTransform=None",      15., 25.,    13., 20. ));
   //                       full low/high , 90 low/high
   TMVA_test.addTest(new RegressionUnitTestWithDeviation( TMVA::Types::kMLP, "MLPBFGSN", "!H:!V:VarTransform=Norm:NeuronType=tanh:NCycles=300:HiddenLayers=N+20:TestRate=6:TrainingMethod=BFGS:Sampling=0.3:SamplingEpoch=0.8:ConvergenceImprove=1e-7:ConvergenceTests=15:!UseRegulator:VarTransform=N" , 0.4, 0.6, 0.3, 0.45 ));
   if (full) TMVA_test.addTest(new RegressionUnitTestWithDeviation( TMVA::Types::kBDT, "BDTG","!H:!V:NTrees=1000::BoostType=Grad:Shrinkage=0.3:!UseBaggedGrad:SeparationType=GiniIndex:nCuts=20:nEventsMin=20:NNodesMax=7" ,  5., 8., 3., 5. ));
   TMVA_test.addTest(new RegressionUnitTestWithDeviation( TMVA::Types::kBDT, "BDTG2","!H:!V:NTrees=2000::BoostType=Grad:Shrinkage=0.1:UseBaggedGrad:GradBaggingFraction=0.5:nCuts=20:MaxDepth=3:NNodesMax=15" ,  2.5, 4., 1.5, 2.5 ));

   if (!full) return;

   TMVA_test.addTest(new RegressionUnitTestWithDeviation( TMVA::Types::kPDERS, "PDERS", "!H:!V:NormTree=T:VolumeRangeMode=Adaptive:KernelEstimator=Gauss:GaussSigma=1.0:NEventsMin=10:NEventsMax=60:VarTransform=None", 10., 15., 5., 10. ));
   TMVA_test.addTest(new RegressionUnitTestWithDeviation( TMVA::Types::kPDERS, "PDERSkNN", "!H:!V:VolumeRangeMode=kNN:KernelEstimator=Gauss:GaussSigma=1.0:NEventsMin=10:NEventsMax=60" , 10., 15., 5., 10. ));
   TMVA_test.addTest(new RegressionUnitTestWithDeviation( TMVA::Types::kPDEFoam, "PDEFoam", "!H:!V:MultiTargetRegression=F:TargetSelection=Mpv:TailCut=0.001:VolFrac=0.0666:nActiveCells=500:nSampl=5000:nBin=7:Compress=T:Kernel=Gauss:Nmin=10:VarTransform=None", 10., 15., 4., 6. ));
   TMVA_test.addTest(new RegressionUnitTestWithDeviation( TMVA::Types::kKNN, "KNN",  "nkNN=20:ScaleFrac=0.8:SigmaFact=1.0:Kernel=Gaus:UseKernel=F:UseWeight=T:!Trim" , 9., 12., 5., 6. ));
   TString baseRegFDAstring="!H:!V:Formula=(0)+(1)*x0+(2)*x1+(3)*x0*x1+(4)*x0*x0+(5)*x1*x1:ParRanges=(-1,1);(-2,2);(-2,2);(-2,2);(-2,2);(-2,2):";
   TMVA_test.addTest(new RegressionUnitTestWithDeviation( TMVA::Types::kFDA, "FDA_MC", baseRegFDAstring+"FitMethod=MC:SampleSize=1000:Sigma=0.1:VarTransform=N", 15., 25., 10., 20. ));
   TMVA_test.addTest(new RegressionUnitTestWithDeviation( TMVA::Types::kFDA, "FDA_GA", baseRegFDAstring+"FitMethod=GA:PopSize=100:Cycles=3:Steps=30:Trim=True:SaveBestGen=1:VarTransform=Norm",  0.5, 1.0, 0.5 , 0.8 ));
   TMVA_test.addTest(new RegressionUnitTestWithDeviation( TMVA::Types::kFDA, "FDA_MT", baseRegFDAstring+"FitMethod=MINUIT:ErrorLevel=1:PrintLevel=-1:FitStrategy=2:UseImprove:UseMinos:SetBatch" , 100., 250., 100., 200. ));
   TMVA_test.addTest(new RegressionUnitTestWithDeviation( TMVA::Types::kFDA, "FDA_GAMT",baseRegFDAstring+"FitMethod=GA:Converger=MINUIT:ErrorLevel=1:PrintLevel=-1:FitStrategy=0:UseImprove:UseMinos:SetBatch:Cycles=1:PopSize=20:Steps=5:Trim" ,  100., 250., 100., 220. ));
   baseRegFDAstring="!H:!V:Formula=(0)+(1)*x0+(2)*x1+(3)*x0*x1+(4)*x0*x0+(5)*x1*x1:ParRanges=(-100,100);(-200,200);(-200,200);(-200,200);(-200,200);(-200,200):";
   TMVA_test.addTest(new RegressionUnitTestWithDeviation( TMVA::Types::kFDA, "FDA_GA", baseRegFDAstring+"FitMethod=GA:PopSize=100:Cycles=3:Steps=30:Trim=True:SaveBestGen=1",  0.3, 0.5, 0.2 , 0.4 ));
   TMVA_test.addTest(new RegressionUnitTestWithDeviation( TMVA::Types::kMLP, "MLPBFGS", "!H:!V:VarTransform=Norm:NeuronType=tanh:NCycles=300:HiddenLayers=N+20:TestRate=6:TrainingMethod=BFGS:Sampling=0.3:SamplingEpoch=0.8:ConvergenceImprove=1e-6:ConvergenceTests=15:!UseRegulator" , 0.4, 0.6, 0.3, 0.4 ));
   // this does not work satisfactory TMVA_test.addTest(new RegressionUnitTestWithDeviation( TMVA::Types::kMLP, "MLP", "!H:!V:VarTransform=Norm:NeuronType=tanh:NCycles=300:HiddenLayers=N+20:TestRate=6:TrainingMethod=BP:Sampling=0.3:SamplingEpoch=0.8:ConvergenceImprove=1e-6:ConvergenceTests=15:!UseRegulator" , 0.4, 0.8, 0.2, 0.5 ));
   // SVM regression does not work TMVA_test.addTest(new RegressionUnitTestWithDeviation( TMVA::Types::kSVM, "SVM", "Gamma=0.25:Tol=0.001:VarTransform=Norm" , 0., 1., 0., 1. ));
   TMVA_test.addTest(new RegressionUnitTestWithDeviation( TMVA::Types::kBDT, "BDT","!H:!V",  8., 10., 5., 8. ));
   TMVA_test.addTest(new RegressionUnitTestWithDeviation( TMVA::Types::kBDT, "BDTN","!H:!V:VarTransform=N",  7., 10., 5., 8. ));
   TMVA_test.addTest(new RegressionUnitTestWithDeviation( TMVA::Types::kBDT, "BDT2","!H:!V:NTrees=500:nEventsMin=20:BoostType=AdaBoostR2:SeparationType=GiniIndex:nCuts=20:PruneMethod=CostComplexity:PruneStrength=3", 12., 17., 10., 14. ));
}

void addDataInputTests( UnitTestSuite& TMVA_test, bool full=true)
{
   if (!full) return;

   TString lhstring = "!H:!V:!TransformOutput:PDFInterpol=Spline2:NSmoothSig[0]=20:NSmoothBkg[0]=20:NSmooth=5:NAvEvtPerBin=50:VarTransform=Decorrelate";

   TMVA_test.addTest(new MethodUnitTestWithComplexData(TString("sig1_bgd1"), TString("nTrain_Signal=500:nTrain_Background=500:SplitMode=random:NormMode=NumEvents:!V"), TMVA::Types::kLikelihood, "LikelihoodD", lhstring , 0.48, 0.52) );
   TMVA_test.addTest(new MethodUnitTestWithComplexData(TString("sig1_bgd1"), TString("nTrain_Signal=500:nTrain_Background=500:SplitMode=block:NormMode=NumEvents:!V"), TMVA::Types::kLikelihood, "LikelihoodD", lhstring , 0.48, 0.52) );
   TMVA_test.addTest(new MethodUnitTestWithComplexData(TString("sig1_bgd1"), TString("nTrain_Signal=500:nTrain_Background=500:SplitMode=alternate:NormMode=NumEvents:!V"), TMVA::Types::kLikelihood, "LikelihoodD", lhstring , 0.48, 0.52) );

   TMVA_test.addTest(new MethodUnitTestWithComplexData(TString("sigfull_bgdfull"), TString("nTrain_Signal=500:nTrain_Background=500:SplitMode=Random:NormMode=NumEvents:!V"), TMVA::Types::kLikelihood, "LikelihoodD", lhstring , 0.9, 0.95) );
   TMVA_test.addTest(new MethodUnitTestWithComplexData(TString("sigfull_bgdfull"), TString("nTrain_Signal=500:nTrain_Background=500:SplitMode=alternate:NormMode=NumEvents:!V"), TMVA::Types::kLikelihood, "LikelihoodD", lhstring , 0.9, 0.95) );
   TMVA_test.addTest(new MethodUnitTestWithComplexData(TString("sigfull_bgdfull"), TString("nTrain_Signal=500:nTrain_Background=500:SplitMode=block:NormMode=NumEvents:!V"), TMVA::Types::kLikelihood, "LikelihoodD", lhstring , 0.9, 0.95) );

   TMVA_test.addTest(new MethodUnitTestWithComplexData(TString("sig1_sig2_bgd1_bgd2"), TString("nTrain_Signal=500:nTrain_Background=500:nTest_Signal=500:nTest_Background=500:SplitMode=random:NormMode=NumEvents:!V"), TMVA::Types::kLikelihood, "LikelihoodD", lhstring , 0.9, 0.95) );
   TMVA_test.addTest(new MethodUnitTestWithComplexData(TString("sig1_sig2_bgd1_bgd2"), TString("nTrain_Signal=500:nTrain_Background=500:nTest_Signal=500:nTest_Background=500:SplitMode=block:NormMode=NumEvents:!V"), TMVA::Types::kLikelihood, "LikelihoodD", lhstring , 0.2, 0.45) );

   TMVA_test.addTest(new MethodUnitTestWithComplexData(TString("sig1_sig2_bgd1_bgd2"), TString("nTrain_Signal=500:nTrain_Background=500:nTest_Signal=500:nTest_Background=500:SplitMode=alternate:NormMode=NumEvents:!V"), TMVA::Types::kLikelihood, "LikelihoodD", lhstring , 0.45, 0.52) );

   TMVA_test.addTest(new MethodUnitTestWithComplexData(TString("sig1_sig2_bgd1_bgd2"), TString("SplitMode=random:NormMode=NumEvents:!V"), TMVA::Types::kLikelihood, "LikelihoodD", lhstring , 0.9, 0.95) );
   TMVA_test.addTest(new MethodUnitTestWithComplexData(TString("sig1_sig2_bgd1_bgd2"), TString("SplitMode=alternate:NormMode=NumEvents:!V"), TMVA::Types::kLikelihood, "LikelihoodD", lhstring , 0.9, 0.95) );
   TMVA_test.addTest(new MethodUnitTestWithComplexData(TString("sig1_sig2_bgd1_bgd2"), TString("SplitMode=block:NormMode=NumEvents:!V"), TMVA::Types::kLikelihood, "LikelihoodD", lhstring , 0.9, 0.994) );
   TMVA_test.addTest(new MethodUnitTestWithComplexData(TString("nvar20_sig1_sig2_bgd1_bgd2"), TString("nTrain_Signal=500:nTrain_Background=500:SplitMode=random:NormMode=NumEvents:!V"), TMVA::Types::kBDT, "BDTDECORR", "NTrees=50:BoostType=Grad:VarTransform=D" , 0.75, 0.98) );
   TMVA_test.addTest(new MethodUnitTestWithComplexData(TString("nvar30_sig1_sig2_bgd1_bgd2"), TString("nTrain_Signal=500:nTrain_Background=500:SplitMode=random:NormMode=NumEvents:!V"), TMVA::Types::kBDT, "BDTDECORR", "NTrees=50:BoostType=Grad:VarTransform=D" , 0.75, 0.98) );
   TMVA_test.addTest(new MethodUnitTestWithComplexData(TString("nvar50_sig1_sig2_bgd1_bgd2"), TString("nTrain_Signal=500:nTrain_Background=500:SplitMode=random:NormMode=NumEvents:!V"), TMVA::Types::kBDT, "BDTNOCORR", "NTrees=50:BoostType=Grad" , 0.48, 0.52) );
   TMVA_test.addTest(new MethodUnitTestWithComplexData(TString("nvar50_sig1_sig2_bgd1_bgd2"), TString("nTrain_Signal=500:nTrain_Background=500:SplitMode=random:NormMode=NumEvents:!V"), TMVA::Types::kBDT, "BDTDECORR", "NTrees=50:BoostType=Grad:VarTransform=D" , 0.75, 0.98) );
}

void addComplexClassificationTests( UnitTestSuite& TMVA_test, bool full=true )
{
   if (!full) return;
   TString trees="sigfull_bgdfull";
   TString prep="nTrain_Signal=2000:nTrain_Background=2000:nTest_Signal=1000:nTest_Background=1000:!V";
   // complex data tests Fisher for comparison
   TMVA_test.addTest(new MethodUnitTestWithComplexData( trees, prep,TMVA::Types::kFisher, "Fisher", "H:!V:VarTransform=Gauss", 0.93, 0.945) );
   // complex data tests with MLP
   TMVA_test.addTest(new MethodUnitTestWithComplexData(trees, prep, TMVA::Types::kMLP, "MLP", "H:!V:RandomSeed=9:NeuronType=tanh:VarTransform=N:NCycles=50:HiddenLayers=N+10:TestRate=5:TrainingMethod=BFGS:!UseRegulator" , 0.955, 0.975) );
   TMVA_test.addTest(new MethodUnitTestWithComplexData(trees, prep, TMVA::Types::kMLP, "MLP", "H:!V:RandomSeed=9:NeuronType=tanh:VarTransform=N:NCycles=50:HiddenLayers=N+10:TestRate=5:TrainingMethod=BP:!UseRegulator" , 0.955, 0.975) );
   // BDT
   TMVA_test.addTest(new MethodUnitTestWithComplexData(trees, prep, TMVA::Types::kBDT, "BDTG8_50", "!H:!V:NTrees=50:BoostType=Grad:Shrinkage=0.30:UseBaggedGrad:GradBaggingFraction=0.6:nCuts=20:NNodesMax=8:SeparationType=GiniIndex" , 0.950, 0.975) );
   //BDT with Fisher cuts
   TMVA_test.addTest(new MethodUnitTestWithComplexData( trees, prep,TMVA::Types::kBDT, "BDTAvecFisher", "!H:V:NTrees=350:NCuts=101:MaxDepth=1:UseFisherCuts:UseExclusiveVars:MinLinCorrForFisher=0.", 0.955, 0.975) );
   // SVM
   TMVA_test.addTest(new MethodUnitTestWithComplexData(trees, prep, TMVA::Types::kSVM, "SVM", "Gamma=0.4:Tol=0.001" , 0.955, 0.975) );

}

void addTestwithSphereTutorial( UnitTestSuite& TMVA_test, bool full=true )
{
   if (!full) return;
   TString trees="sigfull_bgdfull";
   TString prep="nTrain_Signal=2000:nTrain_Background=2000:nTest_Signal=2000:nTest_Background=2000:!V";
   // complex data tests Fisher for comparison
   TMVA_test.addTest(new utSphereData( trees, prep,TMVA::Types::kFisher, "Fisher", "H:!V:VarTransform=Gauss", 0.79, 0.81) );
   // complex data tests with MLP
   TMVA_test.addTest(new utSphereData(trees, prep, TMVA::Types::kMLP, "MLP", "H:!V:RandomSeed=9:NeuronType=tanh:VarTransform=N:NCycles=50:HiddenLayers=N+20:TestRate=5:TrainingMethod=BFGS:!UseRegulator" , 0.95, 0.97) );
   TMVA_test.addTest(new utSphereData(trees, prep, TMVA::Types::kMLP, "MLPBNN", "H:!V:RandomSeed=9:NeuronType=tanh:VarTransform=N:NCycles=50:HiddenLayers=N+20:TestRate=5:TrainingMethod=BFGS:UseRegulator" , 0.95, 0.97) );
   TMVA_test.addTest(new utSphereData(trees, prep, TMVA::Types::kMLP, "MLP_BP", "H:!V:RandomSeed=9:NeuronType=tanh:VarTransform=N:NCycles=50:HiddenLayers=N+20:TestRate=5:TrainingMethod=BP:!UseRegulator" , 0.95, 0.975) );
   // BDT
   TMVA_test.addTest(new utSphereData(trees, prep, TMVA::Types::kBDT, "BDTG", "!H:!V:NTrees=100:BoostType=Grad:Shrinkage=0.30:UseBaggedGrad:GradBaggingFraction=0.6:nCuts=20:NNodesMax=8:SeparationType=GiniIndex" , 0.96, 0.98) );
   TMVA_test.addTest(new utSphereData(trees, prep, TMVA::Types::kBDT, "BDT", "!H:!V:NTrees=100:Shrinkage=0.30:UseBaggedGrad:GradBaggingFraction=0.6:nCuts=20:NNodesMax=8:SeparationType=GiniIndex" , 0.96, 0.98) );
   // SVM
   TMVA_test.addTest(new utSphereData(trees, prep, TMVA::Types::kSVM, "SVM", "Gamma=0.4:Tol=0.001" , 0.97, 0.988 ) );
}

void addGeneralBoostTests(UnitTestSuite& TMVA_test, bool full=true )
{
   if (!full) return;
   for (int i=1;i<=16;i+=5)
      TMVA_test.addTest(new MethodUnitTestWithROCLimits( 
      TMVA::Types::kMLP, Form("BoostedMLP_%d",i),
      Form("H:!V:Boost_Num=%d:Boost_Transform=step:Boost_Type=AdaBoost:NCycles=20:HiddenLayers=N,N+5:Boost_AdaBoostBeta=0.5",i), 0.9-0.3/i, 0.98));

   TMVA_test.addTest(new MethodUnitTestWithROCLimits( 
   TMVA::Types::kMLP, "BoostedMLP",
   "H:!V:Boost_Num=10:Boost_Transform=step:Boost_Type=AdaBoost:NCycles=20:HiddenLayers=N,N+5:NeuronType=sigmoid:Boost_AdaBoostBeta=1.0", 0.8, 0.93));

   TMVA_test.addTest(new MethodUnitTestWithROCLimits( 
   TMVA::Types::kFisher, "BoostedFisher",
   "H:!V:Boost_Num=10:Boost_Transform=log:Boost_Type=AdaBoost:Boost_AdaBoostBeta=0.5", 0.8, 0.95));

   TMVA_test.addTest(new MethodUnitTestWithROCLimits( 
   TMVA::Types::kFisher, "BoostedFisher_step",
   "H:!V:Boost_Num=10:Boost_Transform=step:Boost_Type=AdaBoost:Boost_AdaBoostBeta=0.5", 0.85, 0.92));

   TString trees="sigfull_bgdfull";
   TString prep="nTrain_Signal=2000:nTrain_Background=2000:nTest_Signal=1000:nTest_Background=1000:!V";
   // complex data tests boosted Fisher 
   TMVA_test.addTest(new MethodUnitTestWithComplexData( trees, prep,TMVA::Types::kFisher, "Fisher", "H:!V:Boost_Num=50:Boost_Transform=step:Boost_Type=AdaBoost:Boost_AdaBoostBeta=0.5", 0.9, 0.955) );
   TMVA_test.addTest(new MethodUnitTestWithComplexData( trees, prep,TMVA::Types::kMLP, "MLP", "H:!V:Boost_Num=20:Boost_Transform=step:Boost_Type=AdaBoost:Boost_AdaBoostBeta=1.0:NCycles=20:HiddenLayers=N,N+5", 0.93, 0.985) );

   TMVA_test.addTest(new MethodUnitTestWithROCLimits(
   TMVA::Types::kPDEFoam, "BoostedPDEFoam",
   "H:!V:Boost_Num=10:Boost_Transform=linear:SigBgSeparate=F:TailCut=0.001:nActiveCells=500:nBin=5:Nmin=100:Kernel=None:Compress=T", 0.83, 0.90));

   TMVA_test.addTest(new MethodUnitTestWithROCLimits(
   TMVA::Types::kPDEFoam, "BoostedDTPDEFoam",
   "H:!V:Boost_Num=30:Boost_Transform=linear:SigBgSeparate=F:MaxDepth=4:UseYesNoCell=T:DTLogic=MisClassificationError:FillFoamWithOrigWeights=F:TailCut=0:nActiveCells=500:nBin=20:Nmin=400:Kernel=None:Compress=T", 0.83, 0.90));

}

void addEarlyVersionReaderTests(UnitTestSuite& TMVA_test, bool full=true )
{
   if (!full) return;
   std::vector<TString> methods;
   methods.push_back("Cuts");
   methods.push_back("BDT");
   methods.push_back("BDTG");
   methods.push_back("FDA_GA");
   methods.push_back("KNN");
   methods.push_back("LD");
   methods.push_back("MLP");
   methods.push_back("PDEFoam");
   std::vector<TString> methodsReg;
   methodsReg.push_back("BDTG");
   methods.push_back("FDA_GA");
   methodsReg.push_back("KNN");
   methodsReg.push_back("LD");
   methodsReg.push_back("MLP");
   methodsReg.push_back("PDEFoam");
   std::vector<TString> versions;
   versions.push_back("4.1.2");
   versions.push_back("4.1.0");
   versions.push_back("4.0.6");
   versions.push_back("4.0.3");

   // classification tests
   std::vector<TString>::iterator itmeth=methods.begin();
   for (; itmeth != methods.end();itmeth++){ 
      std::vector<TString>::iterator itvers=versions.begin();
      for (; itvers != versions.end();itvers++){ 
         TMVA_test.addTest( new ReaderTest(
                                                 *itmeth,
                                                 TString("weights")+(*itvers)
                                                 )); 
      }
   }

   bool isRegression=true;
   itmeth=methodsReg.begin();
   for (; itmeth != methodsReg.end();itmeth++){ 
      std::vector<TString>::iterator itvers=versions.begin();
      for (; itvers != versions.end();itvers++){ 
         if (*itmeth=="BDTG" && (*itvers=="4.0.6" || *itvers=="4.0.3") ) continue;
         if (*itmeth=="KNN"  && (*itvers=="4.0.6" || *itvers=="4.0.3") ) continue;

         TMVA_test.addTest( new ReaderTest(
                                                 *itmeth,
                                                 TString("weights")+(*itvers),
                                                 isRegression
                                                 )); 
      }
   }


}

int main(int argc, char **argv)
{

   TApplication theApp("App", &argc, argv);
   gBenchmark = new TBenchmark();
   gBenchmark->Start("stress");
   bool full=false;
#ifdef FULL
   full=true;
   //std::cout << running longer tests<<std::endl;
#endif

   UnitTestSuite TMVA_test("TMVA unit testing");

   TMVA_test.intro();
   TMVA_test.addTest(new utEvent);
   TMVA_test.addTest(new utVariableInfo);
   TMVA_test.addTest(new utDataSetInfo);
   TMVA_test.addTest(new utDataSet);
   TMVA_test.addTest(new utFactory);
   TMVA_test.addTest(new utWeightHandling);
   TMVA_test.addTest(new utReader);

   addClassificationTests(TMVA_test, full);
   addRegressionTests(TMVA_test, full);
   addDataInputTests(TMVA_test, full);
   addComplexClassificationTests(TMVA_test, full);
   addTestwithSphereTutorial(TMVA_test, full);
   addGeneralBoostTests(TMVA_test, full);
   addEarlyVersionReaderTests(TMVA_test,full);
   // run all
   TMVA_test.run();

#ifdef COUTDEBUG
   long int nFail = TMVA_test.report();
   cout << "Total number of failures: " << nFail << endl;
   cout << "************************************************************************************************" << endl;
#else
   TMVA_test.report();
#endif

#ifndef NOCLEANUP
   //FileStat_t stat;
   //if(!gSystem->GetPathInfo("./weights",stat)) {
#ifdef WIN32
   gSystem->Exec("erase /f /q weights\\*.*");
#else
   gSystem->Exec("rm -rf weights/*");
#endif
#endif
   gBenchmark->Stop("stress");
   Bool_t UNIX = strcmp(gSystem->GetName(), "Unix") == 0;
   printf("******************************************************************\n");
   if (UNIX) {
      TString sp = gSystem->GetFromPipe("uname -a");
      sp.Resize(60);
      printf("*  SYS: %s\n",sp.Data());
      if (strstr(gSystem->GetBuildNode(),"Linux")) {
         sp = gSystem->GetFromPipe("lsb_release -d -s");
         printf("*  SYS: %s\n",sp.Data());
      }
      if (strstr(gSystem->GetBuildNode(),"Darwin")) {
         sp  = gSystem->GetFromPipe("sw_vers -productVersion");
         sp += " Mac OS X ";
         printf("*  SYS: %s\n",sp.Data());
      }
   } else {
      const char *os = gSystem->Getenv("OS");
      if (!os) printf("*  SYS: Windows 95\n");
      else     printf("*  SYS: %s %s \n",os,gSystem->Getenv("PROCESSOR_IDENTIFIER"));
   }

   //printf("******************************************************************\n");
   //gBenchmark->Print("stress");
   Float_t ct = gBenchmark->GetCpuTime("stress");
   printf("******************************************************************\n");
   printf("*  CPUTIME   =%6.1f   *  Root%-8s  %d/%d\n",ct       ,gROOT->GetVersion(),gROOT->GetVersionDate(),gROOT->GetVersionTime());
   printf("******************************************************************\n");
}
