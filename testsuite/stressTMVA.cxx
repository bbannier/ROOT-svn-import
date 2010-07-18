// Authors: Christoph Rosemann, Eckhard von Toerne   July 2010
// TMVA unit tests

#include <iostream>
#include "tmvaut/UnitTestSuite.h"

#include "tmvaut/utEvent.h"
#include "tmvaut/utVariableInfo.h"
#include "tmvaut/utDataSetInfo.h"
#include "tmvaut/utDataSet.h"
#include "tmvaut/MethodUnitTestWithROCLimits.h"
#include "TMVA/Types.h"

using namespace UnitTesting;
using namespace std;

int main()
{
   UnitTestSuite TMVA_test("TMVA unit testing");
   
   TMVA_test.intro();
   
   TMVA_test.addTest(new utEvent);
   TMVA_test.addTest(new utVariableInfo);
   TMVA_test.addTest(new utDataSetInfo);
   TMVA_test.addTest(new utDataSet);
   
   TMVA_test.addTest(new MethodUnitTestWithROCLimits( TMVA::Types::kCuts, "Cuts",
   "!H:!V:FitMethod=MC:EffSel:SampleSize=20000:VarProp=FSmart", 0.3, 0.98) );
   TMVA_test.addTest(new MethodUnitTestWithROCLimits( TMVA::Types::kCuts, "CutsD", 
   "!H:!V:FitMethod=MC:EffSel:SampleSize=20000:VarProp=FSmart:VarTransform=Decorrelate" , 0.6, 0.98) );
   TMVA_test.addTest(new MethodUnitTestWithROCLimits( TMVA::Types::kCuts, "CutsPCA", "!H:!V:FitMethod=MC:EffSel:SampleSize=20000:VarProp=FSmart:VarTransform=PCA" , 0.4, 0.98) );
   TMVA_test.addTest(new MethodUnitTestWithROCLimits( TMVA::Types::kCuts, "CutsGA",
   "H:!V:FitMethod=GA:CutRangeMin[0]=-10:CutRangeMax[0]=10:VarProp[1]=FMax:EffSel:Steps=30:Cycles=4:PopSize=500:SC_steps=10:SC_rate=5:SC_factor=0.95" , 0.4, 0.98) );
   TMVA_test.addTest(new MethodUnitTestWithROCLimits( TMVA::Types::kCuts, "CutsSA",
                                                      "!H:!V:FitMethod=SA:EffSel:MaxCalls=150000:KernelTemp=IncAdaptive:InitialTemp=1e+6:MinTemp=1e-6:Eps=1e-10:UseDefaultScale" , 0.4, 0.98) );
  TMVA_test.addTest(new MethodUnitTestWithROCLimits( TMVA::Types::kLikelihood, "Likelihood", 
        "H:!V:TransformOutput:PDFInterpol=Spline2:NSmoothSig[0]=20:NSmoothBkg[0]=20:NSmoothBkg[1]=10:NSmooth=1:NAvEvtPerBin=50" , 0.5, 1.0) ); 
  TMVA_test.addTest(new MethodUnitTestWithROCLimits( TMVA::Types::kLikelihood, "LikelihoodD", 
        "!H:!V:!TransformOutput:PDFInterpol=Spline2:NSmoothSig[0]=20:NSmoothBkg[0]=20:NSmooth=5:NAvEvtPerBin=50:VarTransform=Decorrelate" , 0.5, 1.0) ); 
  TMVA_test.addTest(new MethodUnitTestWithROCLimits( TMVA::Types::kLikelihood, "LikelihoodPCA", 
        "!H:!V:!TransformOutput:PDFInterpol=Spline2:NSmoothSig[0]=20:NSmoothBkg[0]=20:NSmooth=5:NAvEvtPerBin=50:VarTransform=PCA" , 0.5, 1.0) ); 
  TMVA_test.addTest(new MethodUnitTestWithROCLimits( TMVA::Types::kLikelihood, "LikelihoodKDE", 
        "!H:!V:!TransformOutput:PDFInterpol=KDE:KDEtype=Gauss:KDEiter=Adaptive:KDEFineFactor=0.3:KDEborder=None:NAvEvtPerBin=50" , 0.5, 1.0) ); 
  TMVA_test.addTest(new MethodUnitTestWithROCLimits( TMVA::Types::kLikelihood, "LikelihoodMIX", 
        "!H:!V:!TransformOutput:PDFInterpolSig[0]=KDE:PDFInterpolBkg[0]=KDE:PDFInterpolSig[1]=KDE:PDFInterpolBkg[1]=KDE:PDFInterpolSig[2]=Spline2:PDFInterpolBkg[2]=Spline2:PDFInterpolSig[3]=Spline2:PDFInterpolBkg[3]=Spline2:KDEtype=Gauss:KDEiter=Nonadaptive:KDEborder=None:NAvEvtPerBin=50" , 0.5, 1.0) ); 
  TMVA_test.addTest(new MethodUnitTestWithROCLimits( TMVA::Types::kPDERS, "PDERS", 
        "!H:!V:NormTree=T:VolumeRangeMode=Adaptive:KernelEstimator=Gauss:GaussSigma=0.3:NEventsMin=400:NEventsMax=600" , 0.5, 1.0) );
  TMVA_test.addTest(new MethodUnitTestWithROCLimits( TMVA::Types::kPDERS, "PDERSkNN", 
        "!H:!V:VolumeRangeMode=kNN:KernelEstimator=Gauss:GaussSigma=0.3:NEventsMin=400:NEventsMax=600" , 0.5, 1.0) );
  TMVA_test.addTest(new MethodUnitTestWithROCLimits( TMVA::Types::kPDERS, "PDERSD", 
        "!H:!V:VolumeRangeMode=Adaptive:KernelEstimator=Gauss:GaussSigma=0.3:NEventsMin=400:NEventsMax=600:VarTransform=Decorrelate" , 0.5, 1.0) );
  TMVA_test.addTest(new MethodUnitTestWithROCLimits( TMVA::Types::kPDERS, "PDERSPCA", 
        "!H:!V:VolumeRangeMode=Adaptive:KernelEstimator=Gauss:GaussSigma=0.3:NEventsMin=400:NEventsMax=600:VarTransform=PCA" , 0.5, 1.0) );
  TMVA_test.addTest(new MethodUnitTestWithROCLimits( TMVA::Types::kPDEFoam, "PDEFoam", 
        "H:!V:SigBgSeparate=F:TailCut=0.001:VolFrac=0.0333:nActiveCells=500:nSampl=2000:nBin=5:CutNmin=T:Nmin=100:Kernel=None:Compress=T" , 0.5, 1.0) );
  TMVA_test.addTest(new MethodUnitTestWithROCLimits( TMVA::Types::kKNN, "KNN", 
        "H:nkNN=20:ScaleFrac=0.8:SigmaFact=1.0:Kernel=Gaus:UseKernel=F:UseWeight=T:!Trim" , 0.5, 1.0) );
  TMVA_test.addTest(new MethodUnitTestWithROCLimits( TMVA::Types::kHMatrix, "HMatrix", "!H:!V" , 0.5, 1.0) ); 
  TMVA_test.addTest(new MethodUnitTestWithROCLimits( TMVA::Types::kFisher, "Fisher", 
		"H:!V:Fisher:CreateMVAPdfs:PDFInterpolMVAPdf=Spline2:NbinsMVAPdf=60:NsmoothMVAPdf=10" , 0.5, 1.0) );
  TMVA_test.addTest(new MethodUnitTestWithROCLimits( TMVA::Types::kFisher, "FisherG", "H:!V:VarTransform=Gauss" , 0.5, 1.0) );
  TMVA_test.addTest(new MethodUnitTestWithROCLimits( TMVA::Types::kFisher, "BoostedFisher", 
		"H:!V:Boost_Num=20:Boost_Transform=log:Boost_Type=AdaBoost:Boost_AdaBoostBeta=0.2", 0.5, 1.0) );
  TMVA_test.addTest(new MethodUnitTestWithROCLimits(TMVA::Types::kLD, "LD","!H:!V", 0.8, 0.9) );
  TMVA_test.addTest(new MethodUnitTestWithROCLimits( TMVA::Types::kLD, "LD", "H:!V:VarTransform=None" , 0.5, 1.0) );

  TString baseFDAstring="!H:!V:Formula=(0)+(1)*x0+(2)*x1+(3)*x2+(4)*x3:ParRanges=(-1,1);(-10,10);(-10,10);(-10,10);(-10,10):";
  TMVA_test.addTest(new MethodUnitTestWithROCLimits( TMVA::Types::kFDA, "FDA_MC",
  baseFDAstring+"FitMethod=MC:SampleSize=5000:Sigma=0.1" , 0.8, 0.98) );
  TMVA_test.addTest(new MethodUnitTestWithROCLimits( TMVA::Types::kFDA, "FDA_GA",
  baseFDAstring+"FitMethod=GA:PopSize=300:Cycles=3:Steps=20:Trim=True:SaveBestGen=1" , 0.8, 0.98) );
  TMVA_test.addTest(new MethodUnitTestWithROCLimits( TMVA::Types::kFDA, "FDA_SA",
  baseFDAstring+"FitMethod=SA:MaxCalls=5000:KernelTemp=IncAdaptive:InitialTemp=1e+6:MinTemp=1e-6:Eps=1e-10:UseDefaultScale" , 0.8, 0.98) );
  TMVA_test.addTest(new MethodUnitTestWithROCLimits( TMVA::Types::kFDA, "FDA_MT",
  baseFDAstring+"FitMethod=MINUIT:ErrorLevel=1:PrintLevel=-1:FitStrategy=2:UseImprove:UseMinos:SetBatch" , 0.1, 0.98) );
  TMVA_test.addTest(new MethodUnitTestWithROCLimits( TMVA::Types::kFDA, "FDA_GAMT",
  baseFDAstring+"FitMethod=GA:Converger=MINUIT:ErrorLevel=1:PrintLevel=-1:FitStrategy=0:!UseImprove:!UseMinos:SetBatch:Cycles=1:PopSize=5:Steps=5:Trim" , 0.4, 0.98) );
  TMVA_test.addTest(new MethodUnitTestWithROCLimits( TMVA::Types::kFDA, "FDA_MCMT",
  baseFDAstring+"FitMethod=MC:Converger=MINUIT:ErrorLevel=1:PrintLevel=-1:FitStrategy=0:!UseImprove:!UseMinos:SetBatch:SampleSize=20" , 0.7, 0.98) );
  TMVA_test.addTest(new MethodUnitTestWithROCLimits( TMVA::Types::kMLP, "MLP", "H:!V:NeuronType=tanh:VarTransform=N:NCycles=600:HiddenLayers=N+5:TestRate=5:!UseRegulator" , 0.5, 1.0) );
  TMVA_test.addTest(new MethodUnitTestWithROCLimits( TMVA::Types::kMLP, "MLPBFGS", "H:!V:NeuronType=tanh:VarTransform=N:NCycles=600:HiddenLayers=N+5:TestRate=5:TrainingMethod=BFGS:!UseRegulator" , 0.5, 1.0) );
  TMVA_test.addTest(new MethodUnitTestWithROCLimits( TMVA::Types::kMLP, "MLPBNN", "H:!V:NeuronType=tanh:VarTransform=N:NCycles=600:HiddenLayers=N+5:TestRate=5:TrainingMethod=BFGS:UseRegulator" , 0.5, 1.0) ); // BFGS training with bayesian regulators
  TMVA_test.addTest(new MethodUnitTestWithROCLimits( TMVA::Types::kCFMlpANN, "CFMlpANN", "!H:!V:NCycles=2000:HiddenLayers=N+1,N"  , 0.5, 1.0) ); // n_cycles:#nodes:#nodes:...  
  TMVA_test.addTest(new MethodUnitTestWithROCLimits( TMVA::Types::kTMlpANN, "TMlpANN", "!H:!V:NCycles=200:HiddenLayers=N+1,N:LearningMethod=BFGS:ValidationFraction=0.3"  , 0.5, 1.0) ); // n_cycles:#nodes:#nodes:...
  TMVA_test.addTest(new MethodUnitTestWithROCLimits( TMVA::Types::kSVM, "SVM", "Gamma=0.25:Tol=0.001:VarTransform=Norm" , 0.5, 1.0) );
  TMVA_test.addTest(new MethodUnitTestWithROCLimits( TMVA::Types::kBDT, "BDTG", 
                       "!H:!V:NTrees=1000:BoostType=Grad:Shrinkage=0.30:UseBaggedGrad:GradBaggingFraction=0.6:SeparationType=GiniIndex:nCuts=20:NNodesMax=5" , 0.5, 1.0) );
  TMVA_test.addTest(new MethodUnitTestWithROCLimits( TMVA::Types::kBDT, "BDT", 
                          "!H:!V:NTrees=400:nEventsMin=400:MaxDepth=3:BoostType=AdaBoost:SeparationType=GiniIndex:nCuts=20:PruneMethod=NoPruning" , 0.5, 1.0) );
  TMVA_test.addTest(new MethodUnitTestWithROCLimits( TMVA::Types::kBDT, "BDTB", 
                           "!H:!V:NTrees=400:BoostType=Bagging:SeparationType=GiniIndex:nCuts=20:PruneMethod=NoPruning" , 0.5, 1.0) );
  TMVA_test.addTest(new MethodUnitTestWithROCLimits( TMVA::Types::kBDT, "BDTD", 
                           "!H:!V:NTrees=400:nEventsMin=400:MaxDepth=3:BoostType=AdaBoost:SeparationType=GiniIndex:nCuts=20:PruneMethod=NoPruning:VarTransform=Decorrelate" , 0.5, 1.0) );
  TMVA_test.addTest(new MethodUnitTestWithROCLimits( TMVA::Types::kRuleFit, "RuleFit",
                           "H:!V:RuleFitModule=RFTMVA:Model=ModRuleLinear:MinImp=0.001:RuleMinDist=0.001:NTrees=20:fEventsMin=0.01:fEventsMax=0.5:GDTau=-1.0:GDTauPrec=0.01:GDStep=0.01:GDNSteps=10000:GDErrScale=1.02" , 0.5, 1.0) );
   
  TMVA_test.run();
  
  long int nFail = TMVA_test.report();
  cout << "Total number of failures: " << nFail << endl;
  cout << "************************************************************************************************" << endl;
   //  return eventTest.report();
}
