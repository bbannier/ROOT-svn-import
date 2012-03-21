/////////////////////////////////////////////////////////////////////////
//
// 'BASIC FUNCTIONALITY' RooFit tutorial macro #101
// 
// Fitting, plotting, toy data generation on one-dimensional p.d.f
//
// pdf = gauss(x,m,s) 
//
//
// 07/2008 - Wouter Verkerke 
// 
/////////////////////////////////////////////////////////////////////////

#ifndef __CINT__
#include "RooGlobalFunc.h"
#endif
#include "RooRealVar.h"
#include "RooDataSet.h"
#include "RooGaussian.h"
#include "TCanvas.h"
#include "RooPlot.h"
#include "RooUnitTest.h"
#include "RooStats/NumberCountingUtils.h"
#include "RooStats/RooStatsUtils.h"

using namespace RooFit;
using namespace RooStats;


class TestBasic101 : public RooUnitTest
{
public: 
   TestBasic101(TFile* refFile, Bool_t writeRef, Int_t verbose) : RooUnitTest("Zbi and Zgamma", refFile, writeRef, verbose) {};
   
Bool_t testCode() {
    
   // Make model for prototype on/off problem
   // Pois(x | s+b) * Pois(y | tau b )
   // for Z_Gamma, use uniform prior on b.
   RooWorkspace* w = new RooWorkspace("w", kTRUE);
   w->factory("Poisson::px(x[150,0,500],sum::splusb(s[0,0,100],b[100,0,300]))");
   w->factory("Poisson::py(y[100,0,500],prod::taub(tau[1.],b))");	     
   w->factory("Uniform::prior_b(b)");

   // construct the Bayesian-averaged model (eg. a projection pdf)
   // p'(x|s) = \int db p(x|s+b) * [ p(y|b) * prior(b) ]
   w->factory("PROJ::averagedModel(PROD::foo(px|b,py,prior_b),b)") ;

   // plot it, blue is averaged model, red is b known exactly
   RooPlot* frame = w->var("x")->frame();
   w->pdf("averagedModel")->plotOn(frame);
   w->pdf("px")->plotOn(frame,LineColor(kRed));
    
   // compare analytic calculation of Z_Bi
   // with the numerical RooFit implementation of Z_Gamma
   // for an example with x = 150, y = 100
    
   // numeric RooFit Z_Gamma
   w->var("y")->setVal(100);
   w->var("x")->setVal(150);
   RooAbsReal* cdf = w->pdf("averagedModel")->createCdf(*w->var("x"));
   cdf->getVal(); // get ugly print messages out of the way
    
   Double_t hybrid_p_value = cdf->getVal() ;
   Double_t zgamma_signif = PValueToSignificance(1-cdf->getVal()) ;

   // analytic Z_Bi
   Double_t Z_Bi = NumberCountingUtils::BinomialWithTauObsZ(150, 100, 1);

   // Register output quantities for regression test
   regPlot(frame,"rs101_zbi_zgamma") ;
   regValue(hybrid_p_value,"rs101_hybrid_p_value") ;
   regValue(zgamma_signif,"rs101_zgamma_signif") ;
   regValue(Z_Bi,"rs101_Z_Bi") ;
    
//   delete w; // interesting why it doesn't work

   return kTRUE ;
  }
} ;




///////////////////////////////////////////////////////////////////////////////
//
// 'LIKELIHOOD PROFILE' RooStats Tutorial Macro #102
//
// Working with the profile likelihood estimator
//
// 03/2012 - Ioan Gabriel Bucur
//
///////////////////////////////////////////////////////////////////////////////

#include "RooStats/ProfileLikelihoodCalculator.h"
#include "RooStats/LikelihoodInterval.h"
#include "RooStats/LikelihoodIntervalPlot.h"

class TestBasic102 : public RooUnitTest
{
public: 
   TestBasic102(TFile* refFile, Bool_t writeRef, Int_t verbose) : RooUnitTest("Profile Likelihood Operator", refFile, writeRef, verbose) {} ;
  
Bool_t testCode() {    

   // Create model and dataset
	RooWorkspace* w = new RooWorkspace("w", kTRUE);


   w->factory("Gaussian::g(x[-10,10], mean[0,-10,10], sigma[2])");
   RooAbsPdf *g = w->pdf("g");
   RooRealVar *x = w->var("x");
   RooRealVar *mean = w->var("mean");

   RooDataSet *data = g->generate(*x, 10000);
   RooArgSet *params = new RooArgSet(); params->add(*mean);

   RooPlot *frame = x->frame();
   g->plotOn(frame);

   ProfileLikelihoodCalculator *pl = new ProfileLikelihoodCalculator(*data,*g, *params);
   pl->SetConfidenceLevel(0.95);

   LikelihoodInterval *interval = pl->GetInterval();
//   cout << "\n95\% interval on " << mean->GetName() << " is :: [" <<
//      interval->LowerLimit(*mean) << ", " << interval->UpperLimit(*mean) << "] " << endl;


//   cout << "Making a plot of the profile likelihood function ... (if it is taking a lot of time use less points or the TF1 drawing option)\n";
   LikelihoodIntervalPlot *plot = new LikelihoodIntervalPlot(interval);
//   plot->SetNPoints(50);
//  plot->Draw(""); // use option TF1 if too slow
   // WARNING: Can I eliminate TCanvas message?

   // Register output quantities for regressiont test
   regValue(interval->LowerLimit(*mean), "rs102_lower_limit_mean");
   regValue(interval->UpperLimit(*mean), "rs102_upper_limit_mean");
   regPlot(frame, "rs102_gaussian_plot");  


   // Cleanup created objects; 
   delete data;
   delete params;
   delete pl;
   delete interval;
   delete plot;
//   delete w; // the plot is the problem


    return kTRUE ;
  }
} ;

class TestBasic103 : public RooUnitTest
{
public: 
  TestBasic103(TFile* refFile, Bool_t writeRef, Int_t verbose) : RooUnitTest("DUMMY",refFile,writeRef,verbose) {} ;
  Bool_t testCode() {    
    return kTRUE ;
  }
} ;

class TestBasic104 : public RooUnitTest
{
public: 
  TestBasic104(TFile* refFile, Bool_t writeRef, Int_t verbose) : RooUnitTest("DUMMY",refFile,writeRef,verbose) {} ;
  Bool_t testCode() {    
    return kTRUE ;
  }
} ;

class TestBasic105 : public RooUnitTest
{
public: 
  TestBasic105(TFile* refFile, Bool_t writeRef, Int_t verbose) : RooUnitTest("DUMMY",refFile,writeRef,verbose) {} ;
  Bool_t testCode() {    
    return kTRUE ;
  }
} ;

