// Authors: Christoph Rosemann, Eckhard von Toerne   July 2010
// TMVA unit tests

#include <iostream>
#include <cstdlib>
#include "tmvaut/UnitTestSuite.h"

#include "tmvaut/utEvent.h"
#include "tmvaut/utVariableInfo.h"
#include "tmvaut/utDataSetInfo.h"
#include "tmvaut/utDataSet.h"
#include "tmvaut/utFactory.h"
#include "tmvaut/utReader.h"
#include "tmvaut/MethodUnitTestWithROCLimits.h"
#include "tmvaut/MethodUnitTestWithComplexData.h"
#include "tmvaut/RegressionUnitTestWithDeviation.h"
#include "TMVA/Types.h"

using namespace UnitTesting;
using namespace std;

void addClassificationTests( UnitTestSuite& TMVA_test, bool full=true)
{
   TMVA_test.addTest(new MethodUnitTestWithROCLimits( TMVA::Types::kCuts, "CutsGA",
                                                      "H:!V:FitMethod=GA:CutRangeMin[0]=-10:CutRangeMax[0]=10:VarProp[1]=FMax:EffSel:Steps=20:Cycles=4:PopSize=300:SC_steps=10:SC_rate=5:SC_factor=0.95" , 0.4, 0.98) );

   if (full){
      TMVA_test.addTest(new MethodUnitTestWithROCLimits( TMVA::Types::kCuts, "Cuts",
                                                         "!H:!V:FitMethod=MC:EffSel:SampleSize=20000:VarProp=FSmart", 0.4, 0.98) );
      TMVA_test.addTest(new MethodUnitTestWithROCLimits( TMVA::Types::kCuts, "CutsD",
                                                         "!H:!V:FitMethod=MC:EffSel:SampleSize=20000:VarProp=FSmart:VarTransform=Decorrelate" , 0.6, 0.98) );
      TMVA_test.addTest(new MethodUnitTestWithROCLimits( TMVA::Types::kCuts, "CutsPCA", "!H:!V:FitMethod=MC:EffSel:SampleSize=20000:VarProp=FSmart:VarTransform=PCA" , 0.4, 0.98) );
      TMVA_test.addTest(new MethodUnitTestWithROCLimits( TMVA::Types::kCuts, "CutsSA",
                                                         "!H:!V:FitMethod=SA:EffSel:MaxCalls=15000:KernelTemp=IncAdaptive:InitialTemp=1e+6:MinTemp=1e-6:Eps=1e-10:UseDefaultScale" , 0.4, 0.98) );
      TMVA_test.addTest(new MethodUnitTestWithROCLimits( TMVA::Types::kLikelihood, "Likelihood",
                                                         "H:!V:TransformOutput:PDFInterpol=Spline2:NSmoothSig[0]=20:NSmoothBkg[0]=20:NSmoothBkg[1]=10:NSmooth=1:NAvEvtPerBin=50" , 0.7, 0.98) );
   }
   TMVA_test.addTest(new MethodUnitTestWithROCLimits( TMVA::Types::kLikelihood, "LikelihoodD",
                                                      "!H:!V:!TransformOutput:PDFInterpol=Spline2:NSmoothSig[0]=20:NSmoothBkg[0]=20:NSmooth=5:NAvEvtPerBin=50:VarTransform=Decorrelate" , 0.88, 0.98) );
   if (full){
      TMVA_test.addTest(new MethodUnitTestWithROCLimits( TMVA::Types::kLikelihood, "LikelihoodPCA",
                                                         "!H:!V:!TransformOutput:PDFInterpol=Spline2:NSmoothSig[0]=20:NSmoothBkg[0]=20:NSmooth=5:NAvEvtPerBin=50:VarTransform=PCA" , 0.88, 0.98) );
      TMVA_test.addTest(new MethodUnitTestWithROCLimits( TMVA::Types::kLikelihood, "LikelihoodMIX",
                                                         "!H:!V:!TransformOutput:PDFInterpolSig[0]=KDE:PDFInterpolBkg[0]=KDE:PDFInterpolSig[1]=KDE:PDFInterpolBkg[1]=KDE:PDFInterpolSig[2]=Spline2:PDFInterpolBkg[2]=Spline2:PDFInterpolSig[3]=Spline2:PDFInterpolBkg[3]=Spline2:KDEtype=Gauss:KDEiter=Nonadaptive:KDEborder=None:NAvEvtPerBin=50" , 0.7, 0.98) );
   }
   TMVA_test.addTest(new MethodUnitTestWithROCLimits( TMVA::Types::kPDERS, "PDERS",
                                                      "!H:!V:NormTree=T:VolumeRangeMode=Adaptive:KernelEstimator=Gauss:GaussSigma=0.1:NEventsMin=100:NEventsMax=600" , 0.8, 0.98) );
   if (full) TMVA_test.addTest(new MethodUnitTestWithROCLimits( TMVA::Types::kPDERS, "PDERSkNN",
                                                                "!H:!V:VolumeRangeMode=kNN:KernelEstimator=Gauss:GaussSigma=0.3:NEventsMin=400:NEventsMax=600" , 0.8, 0.98) );
   if (full) TMVA_test.addTest(new MethodUnitTestWithROCLimits( TMVA::Types::kPDERS, "PDERSD",
                                                                "!H:!V:VolumeRangeMode=Adaptive:KernelEstimator=Gauss:GaussSigma=0.3:NEventsMin=100:NEventsMax=600:VarTransform=Decorrelate" , 0.88, 0.98) );
   if (full) TMVA_test.addTest(new MethodUnitTestWithROCLimits( TMVA::Types::kPDERS, "PDERSPCA",
                                                                "!H:!V:VolumeRangeMode=Adaptive:KernelEstimator=Gauss:GaussSigma=0.3:NEventsMin=100:NEventsMax=600:VarTransform=PCA" , 0.88, 0.98) );
   TMVA_test.addTest(new MethodUnitTestWithROCLimits( TMVA::Types::kPDEFoam, "PDEFoam",
                                                      "H:!V:SigBgSeparate=F:TailCut=0.001:VolFrac=0.0333:nActiveCells=500:nSampl=2000:nBin=5:Nmin=100:Kernel=None:Compress=T" , 0.8, 0.98) );
   TMVA_test.addTest(new MethodUnitTestWithROCLimits( TMVA::Types::kKNN, "KNN",
                                                      "H:nkNN=20:ScaleFrac=0.8:SigmaFact=1.0:Kernel=Gaus:UseKernel=F:UseWeight=T:!Trim" , 0.8, 0.98) );
   if (full) TMVA_test.addTest(new MethodUnitTestWithROCLimits( TMVA::Types::kHMatrix, "HMatrix", "!H:!V:VarTransform=G" , 0.88, 0.98) );
   TMVA_test.addTest(new MethodUnitTestWithROCLimits( TMVA::Types::kFisher, "Fisher",
                                                      "H:!V:Fisher:CreateMVAPdfs:PDFInterpolMVAPdf=Spline2:NbinsMVAPdf=60:NsmoothMVAPdf=10" , 0.88, 0.98) );
   if (full) TMVA_test.addTest(new MethodUnitTestWithROCLimits( TMVA::Types::kFisher, "FisherG", "H:!V:VarTransform=Gauss" , 0.88, 0.98) );
   TMVA_test.addTest(new MethodUnitTestWithROCLimits( TMVA::Types::kFisher, "BoostedFisher",
                                                      "H:!V:Boost_Num=10:Boost_Transform=log:Boost_Type=AdaBoost:Boost_AdaBoostBeta=0.2", 0.88, 0.98) );
   TMVA_test.addTest(new MethodUnitTestWithROCLimits(TMVA::Types::kLD, "LD","!H:!V", 0.88, 0.98) );
   if (full) TMVA_test.addTest(new MethodUnitTestWithROCLimits( TMVA::Types::kLD, "LD2", "H:!V:VarTransform=None" , 0.88, 0.98) );
   if (full) TMVA_test.addTest(new MethodUnitTestWithROCLimits( TMVA::Types::kLD, "LDN", "H:!V:VarTransform=N" , 0.88, 0.98) );

   TString baseFDAstring="!H:!V:Formula=(0)+(1)*x0+(2)*x1+(3)*x2+(4)*x3:ParRanges=(-1,1);(-10,10);(-10,10);(-10,10);(-10,10):";
   if (full) TMVA_test.addTest(new MethodUnitTestWithROCLimits( TMVA::Types::kFDA, "FDA_MC",
                                                                baseFDAstring+"FitMethod=MC:SampleSize=5000:Sigma=0.1" , 0.8, 0.98) );
   if (full) TMVA_test.addTest(new MethodUnitTestWithROCLimits( TMVA::Types::kFDA, "FDA_GA",
                                                                baseFDAstring+"FitMethod=GA:PopSize=100:Cycles=3:Steps=20:Trim=True:SaveBestGen=1" , 0.88, 0.98) );
   if (full) TMVA_test.addTest(new MethodUnitTestWithROCLimits( TMVA::Types::kFDA, "FDA_SA",
                                                                baseFDAstring+"FitMethod=SA:MaxCalls=5000:KernelTemp=IncAdaptive:InitialTemp=1e+6:MinTemp=1e-6:Eps=1e-10:UseDefaultScale" , 0.88, 0.98) );
   TMVA_test.addTest(new MethodUnitTestWithROCLimits( TMVA::Types::kMLP, "MLP", "H:!V:NeuronType=tanh:VarTransform=N:NCycles=200:HiddenLayers=N+5:TestRate=5:!UseRegulator" , 0.88, 0.98) );
   TMVA_test.addTest(new MethodUnitTestWithROCLimits( TMVA::Types::kMLP, "MLPBFGS", "H:!V:NeuronType=tanh:VarTransform=N:NCycles=200:HiddenLayers=N+5:TestRate=5:TrainingMethod=BFGS:!UseRegulator" , 0.88, 0.98) );
   if (full) TMVA_test.addTest(new MethodUnitTestWithROCLimits( TMVA::Types::kMLP, "MLPBNN", "H:!V:NeuronType=tanh:VarTransform=N:NCycles=200:HiddenLayers=N+5:TestRate=5:TrainingMethod=BFGS:UseRegulator" , 0.88, 0.98) ); // BFGS training with bayesian regulators
   if (full) TMVA_test.addTest(new MethodUnitTestWithROCLimits( TMVA::Types::kCFMlpANN, "CFMlpANN", "!H:!V:NCycles=200:HiddenLayers=N+1,N"  , 0.7, 0.98) ); // n_cycles:#nodes:#nodes:...
   if (full) TMVA_test.addTest(new MethodUnitTestWithROCLimits( TMVA::Types::kTMlpANN, "TMlpANN", "!H:!V:NCycles=200:HiddenLayers=N+1,N:LearningMethod=BFGS:ValidationFraction=0.3"  , 0.7, 0.98) ); // n_cycles:#nodes:#nodes:...
   TMVA_test.addTest(new MethodUnitTestWithROCLimits( TMVA::Types::kSVM, "SVM", "Gamma=0.25:Tol=0.001:VarTransform=Norm" , 0.88, 0.98) );
   TMVA_test.addTest(new MethodUnitTestWithROCLimits( TMVA::Types::kBDT, "BDTG",
                                                      "!H:!V:NTrees=400:BoostType=Grad:Shrinkage=0.30:UseBaggedGrad:GradBaggingFraction=0.6:SeparationType=GiniIndex:nCuts=20:NNodesMax=7" , 0.88, 0.98) );
   TMVA_test.addTest(new MethodUnitTestWithROCLimits( TMVA::Types::kBDT, "BDT",
                                                      "!H:!V:NTrees=400:nEventsMin=100:MaxDepth=3:BoostType=AdaBoost:SeparationType=GiniIndex:nCuts=10:PruneMethod=NoPruning" , 0.88, 0.98) );
   if (full) TMVA_test.addTest(new MethodUnitTestWithROCLimits( TMVA::Types::kBDT, "BDTB",
                                                                "!H:!V:NTrees=400:nEventsMin=100:BoostType=Bagging:SeparationType=GiniIndex:nCuts=20:PruneMethod=NoPruning" , 0.8, 0.98) );
   if (full) TMVA_test.addTest(new MethodUnitTestWithROCLimits( TMVA::Types::kBDT, "BDTD",
                                                                "!H:!V:NTrees=400:nEventsMin=200:MaxDepth=3:BoostType=AdaBoost:SeparationType=GiniIndex:nCuts=10:PruneMethod=NoPruning:VarTransform=Decorrelate" , 0.88, 0.98) );
   if (full) TMVA_test.addTest(new MethodUnitTestWithROCLimits( TMVA::Types::kRuleFit, "RuleFit",
                                                                "H:!V:RuleFitModule=RFTMVA:Model=ModRuleLinear:MinImp=0.001:RuleMinDist=0.001:NTrees=20:fEventsMin=0.01:fEventsMax=0.5:GDTau=-1.0:GDTauPrec=0.01:GDStep=0.01:GDNSteps=10000:GDErrScale=1.02" , 0.88, 0.98) );
}

void addRegressionTests( UnitTestSuite& TMVA_test, bool full=true)
{
   TMVA_test.addTest(new RegressionUnitTestWithDeviation( TMVA::Types::kLD, "LD", "!H:!V:VarTransform=None",      15., 25.,    10., 20. ));
   //                       full low/high , 90 low/high
   TMVA_test.addTest(new RegressionUnitTestWithDeviation( TMVA::Types::kMLP, "MLPBFGSN", "!H:!V:VarTransform=Norm:NeuronType=tanh:NCycles=300:HiddenLayers=N+20:TestRate=6:TrainingMethod=BFGS:Sampling=0.3:SamplingEpoch=0.8:ConvergenceImprove=1e-7:ConvergenceTests=15:!UseRegulator:VarTransform=N" , 0.4, 0.8, 0.2, 0.5 ));
   TMVA_test.addTest(new RegressionUnitTestWithDeviation( TMVA::Types::kBDT, "BDTG","!H:!V:NTrees=1000::BoostType=Grad:Shrinkage=0.3:!UseBaggedGrad:SeparationType=GiniIndex:nCuts=20:nEventsMin=20:NNodesMax=7" ,  5., 8., 3., 5. ));
   TMVA_test.addTest(new RegressionUnitTestWithDeviation( TMVA::Types::kBDT, "BDTG2","!H:!V:NTrees=2000::BoostType=Grad:Shrinkage=0.1:UseBaggedGrad:GradBaggingFraction=0.5:nCuts=20:MaxDepth=3:NNodesMax=15" ,  2., 5., 1., 3. ));

   if (!full) return;

   TMVA_test.addTest(new RegressionUnitTestWithDeviation( TMVA::Types::kPDERS, "PDERS", "!H:!V:NormTree=T:VolumeRangeMode=Adaptive:KernelEstimator=Gauss:GaussSigma=1.0:NEventsMin=10:NEventsMax=60:VarTransform=None", 10., 15., 5., 10. ));
   TMVA_test.addTest(new RegressionUnitTestWithDeviation( TMVA::Types::kPDERS, "PDERSkNN", "!H:!V:VolumeRangeMode=kNN:KernelEstimator=Gauss:GaussSigma=1.0:NEventsMin=10:NEventsMax=60" , 10., 15., 5., 10. ));
   TMVA_test.addTest(new RegressionUnitTestWithDeviation( TMVA::Types::kPDEFoam, "PDEFoam", "!H:!V:MultiTargetRegression=F:TargetSelection=Mpv:TailCut=0.001:VolFrac=0.0333:nActiveCells=500:nSampl=5000:nBin=7:Compress=T:Kernel=Gauss:Nmin=10:VarTransform=None", 10., 15., 4., 6. ));
   TMVA_test.addTest(new RegressionUnitTestWithDeviation( TMVA::Types::kKNN, "KNN",  "nkNN=20:ScaleFrac=0.8:SigmaFact=1.0:Kernel=Gaus:UseKernel=F:UseWeight=T:!Trim" , 10., 15., 4., 6. ));
   TString baseRegFDAstring="!H:!V:Formula=(0)+(1)*x0+(2)*x1+(3)*x0*x1+(4)*x0*x0+(5)*x1*x1:ParRanges=(-1,1);(-2,2);(-2,2);(-2,2);(-2,2);(-2,2):";
   TMVA_test.addTest(new RegressionUnitTestWithDeviation( TMVA::Types::kFDA, "FDA_MC", baseRegFDAstring+"FitMethod=MC:SampleSize=1000:Sigma=0.1:VarTransform=N", 15., 25., 10., 20. ));
   TMVA_test.addTest(new RegressionUnitTestWithDeviation( TMVA::Types::kFDA, "FDA_GA", baseRegFDAstring+"FitMethod=GA:PopSize=100:Cycles=3:Steps=30:Trim=True:SaveBestGen=1:VarTransform=Norm",  0.5, 1.0, 0.5 , 0.8 ));
   TMVA_test.addTest(new RegressionUnitTestWithDeviation( TMVA::Types::kFDA, "FDA_MT", baseRegFDAstring+"FitMethod=MINUIT:ErrorLevel=1:PrintLevel=-1:FitStrategy=2:UseImprove:UseMinos:SetBatch" , 100., 250., 100., 200. ));
   TMVA_test.addTest(new RegressionUnitTestWithDeviation( TMVA::Types::kFDA, "FDA_GAMT",baseRegFDAstring+"FitMethod=GA:Converger=MINUIT:ErrorLevel=1:PrintLevel=-1:FitStrategy=0:UseImprove:UseMinos:SetBatch:Cycles=1:PopSize=20:Steps=5:Trim" ,  100., 250., 100., 220. ));
   TMVA_test.addTest(new RegressionUnitTestWithDeviation( TMVA::Types::kMLP, "MLPBFGS", "!H:!V:VarTransform=Norm:NeuronType=tanh:NCycles=300:HiddenLayers=N+20:TestRate=6:TrainingMethod=BFGS:Sampling=0.3:SamplingEpoch=0.8:ConvergenceImprove=1e-6:ConvergenceTests=15:!UseRegulator" , 0.4, 0.8, 0.2, 0.5 ));
   // this does not work satisfactory TMVA_test.addTest(new RegressionUnitTestWithDeviation( TMVA::Types::kMLP, "MLP", "!H:!V:VarTransform=Norm:NeuronType=tanh:NCycles=300:HiddenLayers=N+20:TestRate=6:TrainingMethod=BP:Sampling=0.3:SamplingEpoch=0.8:ConvergenceImprove=1e-6:ConvergenceTests=15:!UseRegulator" , 0.4, 0.8, 0.2, 0.5 ));
   // SVM regression does not work TMVA_test.addTest(new RegressionUnitTestWithDeviation( TMVA::Types::kSVM, "SVM", "Gamma=0.25:Tol=0.001:VarTransform=Norm" , 0., 1., 0., 1. ));
   TMVA_test.addTest(new RegressionUnitTestWithDeviation( TMVA::Types::kBDT, "BDT","!H:!V",  8., 15., 5., 10. ));
   TMVA_test.addTest(new RegressionUnitTestWithDeviation( TMVA::Types::kBDT, "BDTN","!H:!V:VarTransform=N",  8., 15., 5., 10. ));
   TMVA_test.addTest(new RegressionUnitTestWithDeviation( TMVA::Types::kBDT, "BDT2","!H:!V:NTrees=500:nEventsMin=20:BoostType=AdaBoostR2:SeparationType=GiniIndex:nCuts=20:PruneMethod=CostComplexity:PruneStrength=3", 15., 20., 10., 20. ));
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
   TMVA_test.addTest(new MethodUnitTestWithComplexData(TString("sig1_sig2_bgd1_bgd2"), TString("SplitMode=block:NormMode=NumEvents:!V"), TMVA::Types::kLikelihood, "LikelihoodD", lhstring , 0.9, 0.99) );
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
   TMVA_test.addTest(new MethodUnitTestWithComplexData(trees, prep, TMVA::Types::kBDT, "BDTG8_50", "!H:!V:NTrees=50:BoostType=Grad:Shrinkage=0.30:UseBaggedGrad:GradBaggingFraction=0.6:nCuts=20:NNodesMax=8:SeparationType=GiniIndex" , 0.955, 0.975) );
   // SVM
   TMVA_test.addTest(new MethodUnitTestWithComplexData(trees, prep, TMVA::Types::kSVM, "SVM", "Gamma=0.4:Tol=0.001" , 0.955, 0.975) );
}


int main()
{
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
   TMVA_test.addTest(new utReader);

   addClassificationTests(TMVA_test, full);
   addRegressionTests(TMVA_test, full);
   addDataInputTests(TMVA_test, full);
   addComplexClassificationTests(TMVA_test, full);

   // run all
   TMVA_test.run();

   long int nFail = TMVA_test.report();
   cout << "Total number of failures: " << nFail << endl;
   cout << "************************************************************************************************" << endl;
   //  return eventTest.report();
}
