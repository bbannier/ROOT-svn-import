// Authors: Christoph Rosemann, Eckhard von Toerne   July 2010
// TMVA unit tests

#include <iostream>
#include "tmvaut/UnitTestSuite.h"

#include "tmvaut/utEvent.h"
#include "tmvaut/utVariableInfo.h"
#include "tmvaut/utDataSetInfo.h"
#include "tmvaut/utDataSet.h"
#include "tmvaut/MethodUnitTestWithROCLimits.h"
#include "tmvaut/RegressionUnitTestWithDeviation.h"
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
   "!H:!V:FitMethod=MC:EffSel:SampleSize=20000:VarProp=FSmart", 0.4, 0.98) );
   TMVA_test.addTest(new MethodUnitTestWithROCLimits( TMVA::Types::kCuts, "CutsD", 
   "!H:!V:FitMethod=MC:EffSel:SampleSize=20000:VarProp=FSmart:VarTransform=Decorrelate" , 0.6, 0.98) );
   TMVA_test.addTest(new MethodUnitTestWithROCLimits( TMVA::Types::kCuts, "CutsPCA", "!H:!V:FitMethod=MC:EffSel:SampleSize=20000:VarProp=FSmart:VarTransform=PCA" , 0.4, 0.98) );
   TMVA_test.addTest(new MethodUnitTestWithROCLimits( TMVA::Types::kCuts, "CutsGA",
   "H:!V:FitMethod=GA:CutRangeMin[0]=-10:CutRangeMax[0]=10:VarProp[1]=FMax:EffSel:Steps=20:Cycles=4:PopSize=300:SC_steps=10:SC_rate=5:SC_factor=0.95" , 0.4, 0.98) );
   TMVA_test.addTest(new MethodUnitTestWithROCLimits( TMVA::Types::kCuts, "CutsSA",
                                                      "!H:!V:FitMethod=SA:EffSel:MaxCalls=15000:KernelTemp=IncAdaptive:InitialTemp=1e+6:MinTemp=1e-6:Eps=1e-10:UseDefaultScale" , 0.4, 0.98) );
  TMVA_test.addTest(new MethodUnitTestWithROCLimits( TMVA::Types::kLikelihood, "Likelihood", 
        "H:!V:TransformOutput:PDFInterpol=Spline2:NSmoothSig[0]=20:NSmoothBkg[0]=20:NSmoothBkg[1]=10:NSmooth=1:NAvEvtPerBin=50" , 0.7, 0.98) ); 
  TMVA_test.addTest(new MethodUnitTestWithROCLimits( TMVA::Types::kLikelihood, "LikelihoodD", 
        "!H:!V:!TransformOutput:PDFInterpol=Spline2:NSmoothSig[0]=20:NSmoothBkg[0]=20:NSmooth=5:NAvEvtPerBin=50:VarTransform=Decorrelate" , 0.88, 0.98) ); 
  TMVA_test.addTest(new MethodUnitTestWithROCLimits( TMVA::Types::kLikelihood, "LikelihoodPCA", 
        "!H:!V:!TransformOutput:PDFInterpol=Spline2:NSmoothSig[0]=20:NSmoothBkg[0]=20:NSmooth=5:NAvEvtPerBin=50:VarTransform=PCA" , 0.88, 0.98) ); 
  //  TMVA_test.addTest(new MethodUnitTestWithROCLimits( TMVA::Types::kLikelihood, "LikelihoodKDE", 
  //      "!H:!V:!TransformOutput:PDFInterpol=KDE:KDEtype=Gauss:KDEiter=Adaptive:KDEFineFactor=0.3:KDEborder=None:NAvEvtPerBin=50" , 0.7, 0.98) ); 
  TMVA_test.addTest(new MethodUnitTestWithROCLimits( TMVA::Types::kLikelihood, "LikelihoodMIX", 
        "!H:!V:!TransformOutput:PDFInterpolSig[0]=KDE:PDFInterpolBkg[0]=KDE:PDFInterpolSig[1]=KDE:PDFInterpolBkg[1]=KDE:PDFInterpolSig[2]=Spline2:PDFInterpolBkg[2]=Spline2:PDFInterpolSig[3]=Spline2:PDFInterpolBkg[3]=Spline2:KDEtype=Gauss:KDEiter=Nonadaptive:KDEborder=None:NAvEvtPerBin=50" , 0.7, 0.98) ); 
  TMVA_test.addTest(new MethodUnitTestWithROCLimits( TMVA::Types::kPDERS, "PDERS", 
        "!H:!V:NormTree=T:VolumeRangeMode=Adaptive:KernelEstimator=Gauss:GaussSigma=0.1:NEventsMin=100:NEventsMax=600" , 0.8, 0.98) );
  TMVA_test.addTest(new MethodUnitTestWithROCLimits( TMVA::Types::kPDERS, "PDERSkNN", 
        "!H:!V:VolumeRangeMode=kNN:KernelEstimator=Gauss:GaussSigma=0.3:NEventsMin=400:NEventsMax=600" , 0.8, 0.98) );
  TMVA_test.addTest(new MethodUnitTestWithROCLimits( TMVA::Types::kPDERS, "PDERSD", 
        "!H:!V:VolumeRangeMode=Adaptive:KernelEstimator=Gauss:GaussSigma=0.3:NEventsMin=100:NEventsMax=600:VarTransform=Decorrelate" , 0.88, 0.98) );
  TMVA_test.addTest(new MethodUnitTestWithROCLimits( TMVA::Types::kPDERS, "PDERSPCA", 
        "!H:!V:VolumeRangeMode=Adaptive:KernelEstimator=Gauss:GaussSigma=0.3:NEventsMin=100:NEventsMax=600:VarTransform=PCA" , 0.88, 0.98) );
  TMVA_test.addTest(new MethodUnitTestWithROCLimits( TMVA::Types::kPDEFoam, "PDEFoam", 
        "H:!V:SigBgSeparate=F:TailCut=0.001:VolFrac=0.0333:nActiveCells=500:nSampl=2000:nBin=5:Nmin=100:Kernel=None:Compress=T" , 0.8, 0.98) );
  TMVA_test.addTest(new MethodUnitTestWithROCLimits( TMVA::Types::kKNN, "KNN", 
        "H:nkNN=20:ScaleFrac=0.8:SigmaFact=1.0:Kernel=Gaus:UseKernel=F:UseWeight=T:!Trim" , 0.8, 0.98) );
  TMVA_test.addTest(new MethodUnitTestWithROCLimits( TMVA::Types::kHMatrix, "HMatrix", "!H:!V:VarTransform=G" , 0.88, 0.98) ); 
  TMVA_test.addTest(new MethodUnitTestWithROCLimits( TMVA::Types::kFisher, "Fisher", 
		"H:!V:Fisher:CreateMVAPdfs:PDFInterpolMVAPdf=Spline2:NbinsMVAPdf=60:NsmoothMVAPdf=10" , 0.88, 0.98) );
  TMVA_test.addTest(new MethodUnitTestWithROCLimits( TMVA::Types::kFisher, "FisherG", "H:!V:VarTransform=Gauss" , 0.88, 0.98) ); 
  TMVA_test.addTest(new MethodUnitTestWithROCLimits( TMVA::Types::kFisher, "BoostedFisher", 
		"H:!V:Boost_Num=5:Boost_Transform=log:Boost_Type=AdaBoost:Boost_AdaBoostBeta=0.2", 0.88, 0.98) );
  TMVA_test.addTest(new MethodUnitTestWithROCLimits(TMVA::Types::kLD, "LD","!H:!V", 0.88, 0.98) );
  TMVA_test.addTest(new MethodUnitTestWithROCLimits( TMVA::Types::kLD, "LD", "H:!V:VarTransform=None" , 0.88, 0.98) );

  TString baseFDAstring="!H:!V:Formula=(0)+(1)*x0+(2)*x1+(3)*x2+(4)*x3:ParRanges=(-1,1);(-10,10);(-10,10);(-10,10);(-10,10):";
  TMVA_test.addTest(new MethodUnitTestWithROCLimits( TMVA::Types::kFDA, "FDA_MC",
  baseFDAstring+"FitMethod=MC:SampleSize=5000:Sigma=0.1" , 0.8, 0.98) );
  TMVA_test.addTest(new MethodUnitTestWithROCLimits( TMVA::Types::kFDA, "FDA_GA",
  baseFDAstring+"FitMethod=GA:PopSize=300:Cycles=3:Steps=20:Trim=True:SaveBestGen=1" , 0.88, 0.98) );
  TMVA_test.addTest(new MethodUnitTestWithROCLimits( TMVA::Types::kFDA, "FDA_SA",
  baseFDAstring+"FitMethod=SA:MaxCalls=5000:KernelTemp=IncAdaptive:InitialTemp=1e+6:MinTemp=1e-6:Eps=1e-10:UseDefaultScale" , 0.88, 0.98) );
  //TMVA_test.addTest(new MethodUnitTestWithROCLimits( TMVA::Types::kFDA, "FDA_MT",
  //baseFDAstring+"FitMethod=MINUIT:ErrorLevel=1:PrintLevel=-1:FitStrategy=2:UseImprove:UseMinos:SetBatch" , 0.1, 0.98) );
  //TMVA_test.addTest(new MethodUnitTestWithROCLimits( TMVA::Types::kFDA, "FDA_GAMT",
  //baseFDAstring+"FitMethod=GA:Converger=MINUIT:ErrorLevel=1:PrintLevel=-1:FitStrategy=0:!UseImprove:!UseMinos:SetBatch:Cycles=2:PopSize=5:Steps=5:Trim" , 0.8, 0.98) );
  //TMVA_test.addTest(new MethodUnitTestWithROCLimits( TMVA::Types::kFDA, "FDA_MCMT",
  //baseFDAstring+"FitMethod=MC:Converger=MINUIT:ErrorLevel=1:PrintLevel=-1:FitStrategy=0:!UseImprove:!UseMinos:SetBatch:SampleSize=100" , 0.8, 0.98) );
  TMVA_test.addTest(new MethodUnitTestWithROCLimits( TMVA::Types::kMLP, "MLP", "H:!V:NeuronType=tanh:VarTransform=N:NCycles=400:HiddenLayers=N+5:TestRate=5:!UseRegulator" , 0.88, 0.98) ); 
  TMVA_test.addTest(new MethodUnitTestWithROCLimits( TMVA::Types::kMLP, "MLPBFGS", "H:!V:NeuronType=tanh:VarTransform=N:NCycles=400:HiddenLayers=N+5:TestRate=5:TrainingMethod=BFGS:!UseRegulator" , 0.88, 0.98) );
  TMVA_test.addTest(new MethodUnitTestWithROCLimits( TMVA::Types::kMLP, "MLPBNN", "H:!V:NeuronType=tanh:VarTransform=N:NCycles=400:HiddenLayers=N+5:TestRate=5:TrainingMethod=BFGS:UseRegulator" , 0.88, 0.98) ); // BFGS training with bayesian regulators
  TMVA_test.addTest(new MethodUnitTestWithROCLimits( TMVA::Types::kCFMlpANN, "CFMlpANN", "!H:!V:NCycles=400:HiddenLayers=N+1,N"  , 0.7, 0.98) ); // n_cycles:#nodes:#nodes:...  
  TMVA_test.addTest(new MethodUnitTestWithROCLimits( TMVA::Types::kTMlpANN, "TMlpANN", "!H:!V:NCycles=500:HiddenLayers=N+1,N:LearningMethod=BFGS:ValidationFraction=0.3"  , 0.7, 0.98) ); // n_cycles:#nodes:#nodes:...
  TMVA_test.addTest(new MethodUnitTestWithROCLimits( TMVA::Types::kSVM, "SVM", "Gamma=0.25:Tol=0.001:VarTransform=Norm" , 0.88, 0.98) );
  TMVA_test.addTest(new MethodUnitTestWithROCLimits( TMVA::Types::kBDT, "BDTG", 
                       "!H:!V:NTrees=400:BoostType=Grad:Shrinkage=0.30:UseBaggedGrad:GradBaggingFraction=0.6:SeparationType=GiniIndex:nCuts=20:NNodesMax=7" , 0.88, 0.98) );
  TMVA_test.addTest(new MethodUnitTestWithROCLimits( TMVA::Types::kBDT, "BDT", 
                          "!H:!V:NTrees=400:nEventsMin=100:MaxDepth=3:BoostType=AdaBoost:SeparationType=GiniIndex:nCuts=10:PruneMethod=NoPruning" , 0.88, 0.98) );
  TMVA_test.addTest(new MethodUnitTestWithROCLimits( TMVA::Types::kBDT, "BDTB", 
                           "!H:!V:NTrees=400:nEventsMin=100:BoostType=Bagging:SeparationType=GiniIndex:nCuts=20:PruneMethod=NoPruning" , 0.8, 0.98) );
  TMVA_test.addTest(new MethodUnitTestWithROCLimits( TMVA::Types::kBDT, "BDTD", 
                           "!H:!V:NTrees=400:nEventsMin=200:MaxDepth=3:BoostType=AdaBoost:SeparationType=GiniIndex:nCuts=10:PruneMethod=NoPruning:VarTransform=Decorrelate" , 0.88, 0.98) );
  TMVA_test.addTest(new MethodUnitTestWithROCLimits( TMVA::Types::kRuleFit, "RuleFit",
                           "H:!V:RuleFitModule=RFTMVA:Model=ModRuleLinear:MinImp=0.001:RuleMinDist=0.001:NTrees=20:fEventsMin=0.01:fEventsMax=0.5:GDTau=-1.0:GDTauPrec=0.01:GDStep=0.01:GDNSteps=10000:GDErrScale=1.02" , 0.88, 0.98) );

  // regression tests
   TMVA_test.addTest(new RegressionUnitTestWithDeviation( TMVA::Types::kLD, "LD", "!H:!V:VarTransform=None", 0., 1., 0., 1. ));          
   TMVA_test.addTest(new RegressionUnitTestWithDeviation( TMVA::Types::kPDERS, "PDERS", "!H:!V:NormTree=T:VolumeRangeMode=Adaptive:KernelEstimator=Gauss:GaussSigma=0.3:NEventsMin=40:NEventsMax=60:VarTransform=None", 0., 1., 0., 1. ));
   TMVA_test.addTest(new RegressionUnitTestWithDeviation( TMVA::Types::kPDERS, "PDERSkNN", "!H:!V:VolumeRangeMode=kNN:KernelEstimator=Gauss:GaussSigma=0.3:NEventsMin=400:NEventsMax=600" , 0., 1., 0., 1. ));
   TMVA_test.addTest(new RegressionUnitTestWithDeviation( TMVA::Types::kPDEFoam, "PDEFoam", "!H:!V:MultiTargetRegression=F:TargetSelection=Mpv:TailCut=0.001:VolFrac=0.0333:nActiveCells=500:nSampl=2000:nBin=5:Compress=T:Kernel=None:Nmin=10:VarTransform=None", 0., 1., 0., 1. ));
   TMVA_test.addTest(new RegressionUnitTestWithDeviation( TMVA::Types::kKNN, "KNN",  "nkNN=20:ScaleFrac=0.8:SigmaFact=1.0:Kernel=Gaus:UseKernel=F:UseWeight=T:!Trim" , 0., 1., 0., 1. ));
   TMVA_test.addTest(new RegressionUnitTestWithDeviation( TMVA::Types::kLD, "LD","!H:!V:VarTransform=None" , 0., 1., 0., 1. ));
  /*
     TMVA_test.addTest(new RegressionUnitTestWithDeviation( TMVA::Types::kFDA, "FDA_MC","!H:!V:Formula=(0)+(1)*x0+(2)*x1:ParRanges=(-100,100);(-100,100);(-100,100):FitMethod=MC:SampleSize=100000:Sigma=0.1:VarTransform=D", 0., 1., 0., 1. )); 
     yields error:
       <FATAL> Deco                     : Inverse transformation for decorrelation transformation not yet implemented. Hence, this transformation cannot be applied together with regression. Please contact the authors if necessary.
     */
   TMVA_test.addTest(new RegressionUnitTestWithDeviation( TMVA::Types::kFDA, "FDA_GA","!H:!V:Formula=(0)+(1)*x0+(2)*x1:ParRanges=(-100,100);(-100,100);(-100,100):FitMethod=GA:PopSize=100:Cycles=3:Steps=30:Trim=True:SaveBestGen=1:VarTransform=Norm", 0., 1., 0., 1. ));;
   TMVA_test.addTest(new RegressionUnitTestWithDeviation( TMVA::Types::kFDA, "FDA_MT","!H:!V:Formula=(0)+(1)*x0+(2)*x1:ParRanges=(-100,100);(-100,100);(-100,100);(-10,10):FitMethod=MINUIT:ErrorLevel=1:PrintLevel=-1:FitStrategy=2:UseImprove:UseMinos:SetBatch" , 0., 1., 0., 1. ));
   TMVA_test.addTest(new RegressionUnitTestWithDeviation( TMVA::Types::kFDA, "FDA_GAMT","!H:!V:Formula=(0)+(1)*x0+(2)*x1:ParRanges=(-100,100);(-100,100);(-100,100):FitMethod=GA:Converger=MINUIT:ErrorLevel=1:PrintLevel=-1:FitStrategy=0:!UseImprove:!UseMinos:SetBatch:Cycles=1:PopSize=5:Steps=5:Trim" , 0., 1., 0., 1. ));
   TMVA_test.addTest(new RegressionUnitTestWithDeviation( TMVA::Types::kMLP, "MLP", "!H:!V:VarTransform=Norm:NeuronType=tanh:NCycles=20000:HiddenLayers=N+20:TestRate=6:TrainingMethod=BFGS:Sampling=0.3:SamplingEpoch=0.8:ConvergenceImprove=1e-6:ConvergenceTests=15:!UseRegulator" , 0., 1., 0., 1. ));
   TMVA_test.addTest(new RegressionUnitTestWithDeviation( TMVA::Types::kSVM, "SVM", "Gamma=0.25:Tol=0.001:VarTransform=Norm" , 0., 1., 0., 1. ));
   TMVA_test.addTest(new RegressionUnitTestWithDeviation( TMVA::Types::kBDT, "BDT","!H:!V:NTrees=100:nEventsMin=5:BoostType=AdaBoostR2:SeparationType=RegressionVariance:nCuts=20:PruneMethod=CostComplexity:PruneStrength=30", 0., 1., 0., 1. )); 
   TMVA_test.addTest(new RegressionUnitTestWithDeviation( TMVA::Types::kBDT, "BDTG","!H:!V:NTrees=1000::BoostType=Grad:Shrinkage=0.3:!UseBaggedGrad:SeparationType=GiniIndex:nCuts=20:NNodesMax=10" , 0., 1., 0., 1. ));
  //
  TMVA_test.run();
  
  long int nFail = TMVA_test.report();
  cout << "Total number of failures: " << nFail << endl;
  cout << "************************************************************************************************" << endl;
   //  return eventTest.report();
}
