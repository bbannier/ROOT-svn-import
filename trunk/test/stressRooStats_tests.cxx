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
#include "TMath.h"
#include "RooPlot.h"
#include "RooUnitTest.h"
#include "RooStats/NumberCountingUtils.h"
#include "RooStats/RooStatsUtils.h"


// DEBUG
#include "Math/MinimizerOptions.h"

using namespace RooFit;
using namespace RooStats;


class TestBasic101 : public RooUnitTest {
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
      w->pdf("px")->plotOn(frame, LineColor(kRed));

      // compare analytic calculation of Z_Bi
      // with the numerical RooFit implementation of Z_Gamma
      // for an example with x = 150, y = 100

      // numeric RooFit Z_Gamma
      w->var("y")->setVal(100);
      w->var("x")->setVal(150);
      RooAbsReal* cdf = w->pdf("averagedModel")->createCdf(*w->var("x"));
      cdf->getVal(); // get ugly print messages out of the way

      Double_t hybrid_p_value = cdf->getVal() ;
      Double_t zgamma_signif = PValueToSignificance(1 - cdf->getVal()) ;

      // analytic Z_Bi
      Double_t Z_Bi = NumberCountingUtils::BinomialWithTauObsZ(150, 100, 1);

      // Register output quantities for regression test
      regPlot(frame, "rs101_zbi_zgamma") ;
      regValue(hybrid_p_value, "rs101_hybrid_p_value") ;
      regValue(zgamma_signif, "rs101_zgamma_signif") ;
      regValue(Z_Bi, "rs101_Z_Bi") ;

//   delete w; // interesting why it doesn't work

      return kTRUE ;
   }
} ;




///////////////////////////////////////////////////////////////////////////////
//
// 'LIKELIHOOD PROFILE' RooStats Tutorial Macro #102
//
// Test working with the profile likelihood estimator on a Gaussian distribution
//
// 03/2012 - Ioan Gabriel Bucur
//
///////////////////////////////////////////////////////////////////////////////

#include "RooStats/ProfileLikelihoodCalculator.h"
#include "RooStats/LikelihoodInterval.h"
#include "RooStats/LikelihoodIntervalPlot.h"

class TestBasic102 : public RooUnitTest {
public:
   TestBasic102(TFile* refFile, Bool_t writeRef, Int_t verbose) : RooUnitTest("Profile Likelihood Interval - Gaussian", refFile, writeRef, verbose) {} ;

   Bool_t testCode() {

      const Double_t alpha = 0.05; // significance level
      const Int_t N = 1; // number of observations

      // Create Gaussian model and dataset
      RooWorkspace* w = new RooWorkspace("w", kTRUE);
      w->factory("Gaussian::gauss(x[-5,5], mean[0,-5,5], sigma[1])");
//    RooDataSet *data = new RooDataSet("data", "data", *w->var("x"));
//    data->add(*w->var("x"));   
      RooDataSet *data = w->pdf("gauss")->generate(*w->var("x"), N); 


      if (_write == kTRUE) {

         // Calculate likelihood interval from data via analytic methods
         Double_t estMean = data->mean(*w->var("x"));
         Double_t intervalHalfWidth = ROOT::Math::normal_quantile_c(alpha / 2.0, w->var("sigma")->getValV() / sqrt(N));
         Double_t lowerLimit = estMean - intervalHalfWidth;
         Double_t upperLimit = estMean + intervalHalfWidth;

         
         // Compare the limits obtained via ProfileLikelihoodCalculator and LikelihoodInterval with analytically estimated values
         regValue(lowerLimit, "rs102_lower_limit_mean");
         regValue(upperLimit, "rs102_upper_limit_mean");

      } else {
              
         // Calculate likelihood interval using the ProfileLikelihoodCalculator and LikelihoodInterval objects
         RooArgSet *params = new RooArgSet();
         params->add(*w->var("mean"));
         ProfileLikelihoodCalculator *plc = new ProfileLikelihoodCalculator(*data, *w->pdf("gauss"), *params);
         plc->SetConfidenceLevel(1 - alpha);

         LikelihoodInterval *interval = plc->GetInterval();

         // Register analytically computed limits in the reference file
         regValue(interval->LowerLimit(*w->var("mean")), "rs102_lower_limit_mean");
         regValue(interval->UpperLimit(*w->var("mean")), "rs102_upper_limit_mean");

         // Cleanup branch objects
         delete params;
         delete plc;
         delete interval;
      }

      // Cleanup function objects
      delete data;
      delete w;

      return kTRUE ;
   }
};

class TestBasic103 : public RooUnitTest {
public:
   TestBasic103(TFile* refFile, Bool_t writeRef, Int_t verbose) : RooUnitTest("Profile Likelihood Interval - Poisson", refFile, writeRef, verbose) {} ;
   
   Double_t vtol() { return 1e-2; }

   Bool_t testCode() {

      const Double_t alpha = 0.32; // significance level

      // RooMsgService::instance().setGlobalKillBelow(RooFit::FATAL);

      // Create Poisson model and dataset
      RooWorkspace* w = new RooWorkspace("w", kTRUE);
      w->factory("Poisson::poiss(x[1e-100,1000], mean[2,1e-100,1000])");

      // We have just one hard-coded value (3)    
      RooRealVar value("x","x", 3);
      const RooArgSet argSet(value);

      RooDataSet *data = new RooDataSet("poissData", "Poisson data", argSet);
      data->add(argSet);

      //cout << "Sum entries: " << data->sumEntries() << " " << datag->sumEntries() << endl;

      if (_write == kTRUE) {

         // Solution of equation -2*[ln(LL(u_min)) - ln(LL(u))] = 0.994458
         // 0.994458 was chosen since a 68% confidence interval corresponds to estimate +/- 0.994458 * sigma
         // SOURCE: Wolfram Alpha
         Double_t lowerLimit = 1.58708;
         Double_t upperLimit = 5.07346;
         
         // Compare the limits obtained via ProfileLikelihoodCalculator and LikelihoodInterval with Wolfram values
         regValue(lowerLimit, "rs103_lower_limit_mean");
         regValue(upperLimit, "rs103_upper_limit_mean");

      } else {
              
         // Calculate likelihood interval using the ProfileLikelihoodCalculator and LikelihoodInterval objects
         RooArgSet *params = new RooArgSet();
         params->add(*w->var("mean"));
         ProfileLikelihoodCalculator *plc = new ProfileLikelihoodCalculator(*data, *w->pdf("poiss"), *params);
         plc->SetConfidenceLevel(1 - alpha);

         LikelihoodInterval *interval = plc->GetInterval();
                
         // cout << "[" << interval->LowerLimit(*w->var("mean")) << " " << interval->UpperLimit(*w->var("mean")) << "]" << endl;

         // Register externally computed limits in the reference file
         regValue(interval->LowerLimit(*w->var("mean")), "rs103_lower_limit_mean");
         regValue(interval->UpperLimit(*w->var("mean")), "rs103_upper_limit_mean");
         // cout << "gil " << interval->LowerLimit(*w->var("mean")) << " giu " << interval->UpperLimit(*w->var("mean")) << endl;

         // Cleanup branch objects
         delete params;
         delete plc;
         delete interval;
      }

      // Cleanup function objects
      delete data;
      delete w;

      return kTRUE ;
   }
} ;

class TestBasic104 : public RooUnitTest {
public:
   TestBasic104(TFile* refFile, Bool_t writeRef, Int_t verbose) : RooUnitTest("DUMMY", refFile, writeRef, verbose) {} ;
   Bool_t testCode() {
      return kTRUE ;
   }
} ;

class TestBasic105 : public RooUnitTest {
public:
   TestBasic105(TFile* refFile, Bool_t writeRef, Int_t verbose) : RooUnitTest("DUMMY", refFile, writeRef, verbose) {} ;
   Bool_t testCode() {
      return kTRUE ;
   }
} ;

