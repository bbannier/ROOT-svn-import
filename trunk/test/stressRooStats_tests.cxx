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
#include "TCanvas.h"
#include "TMath.h"
#include "RooPlot.h"
#include "RooUnitTest.h"
#include "RooStats/NumberCountingUtils.h"
#include "RooStats/RooStatsUtils.h"

using namespace RooFit;
using namespace RooStats;
using namespace ROOT::Math;

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
// 'PROFILE LIKELIHOOD INTERVAL - GAUSSIAN' RooStats Tutorial Macro #102
//
// Test the likelihood interval computed by the profile likelihood calculator
// on a Gaussian distribution. Reference interval limits are computed via
// analytic methods: solve equation 2*(ln(LL(xMax))-ln(LL(x)) = q, where q =
// normal_quantile_c(testSize/2, 1). In the case of a Gaussian distribution, the
// interval limits are equal to: mean +- normal_quantile_c(testSize/2, sigma/sqrt(N)).
//
// 03/2012 - Ioan Gabriel Bucur
//
///////////////////////////////////////////////////////////////////////////////

#include "RooStats/ProfileLikelihoodCalculator.h"
#include "RooStats/LikelihoodInterval.h"
#include "RooStats/LikelihoodIntervalPlot.h"

class TestBasic102 : public RooUnitTest {
public:
   TestBasic102(TFile* refFile, Bool_t writeRef, Int_t verbose) : RooUnitTest("Profile Likelihood Calculator - Gaussian", refFile, writeRef, verbose) {} ;

   Bool_t testCode() {

      const Double_t testSize = 0.05; // significance level
      const Int_t N = 1; // number of observations

      //TODO: see why it fails for a small number of observations

      // Create Gaussian model and dataset
      RooWorkspace* w = new RooWorkspace("w", kTRUE);
      w->factory("Gaussian::gauss(x[-5,5], mean[0,-5,5], sigma[1])");
      RooDataSet *data = w->pdf("gauss")->generate(*w->var("x"), N);


      if (_write == kTRUE) {

         // Calculate likelihood interval from data via analytic methods
         Double_t estMean = data->mean(*w->var("x"));
         Double_t intervalHalfWidth = ROOT::Math::normal_quantile_c(testSize / 2.0, w->var("sigma")->getValV() / sqrt(N));
         Double_t lowerLimit = estMean - intervalHalfWidth;
         Double_t upperLimit = estMean + intervalHalfWidth;


         // Compare the limits obtained via ProfileLikelihoodCalculator with analytically estimated values
         regValue(lowerLimit, "rs102_lower_limit_mean");
         regValue(upperLimit, "rs102_upper_limit_mean");

      } else {

         // Calculate likelihood interval using the ProfileLikelihoodCalculator
         RooArgSet *params = new RooArgSet();
         params->add(*w->var("mean"));
         ProfileLikelihoodCalculator *plc = new ProfileLikelihoodCalculator(*data, *w->pdf("gauss"), *params);
         plc->SetTestSize(testSize);

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



///////////////////////////////////////////////////////////////////////////////
//
// 'PROFILE LIKELIHOOD INTERVAL - POISSON' RooStats Tutorial Macro #103
//
// Test the 68% likelihood interval computed by the profile likelihood calculator
// on a Poisson distribution, from only one observed value. Reference values are
// computed via analytic methods: solve equation 2*[ln(LL(xMax)) - ln(LL(x))] = 1.
//
// 03/2012 - Ioan Gabriel Bucur
//
///////////////////////////////////////////////////////////////////////////////


class TestBasic103 : public RooUnitTest {
public:
   TestBasic103(TFile* refFile, Bool_t writeRef, Int_t verbose, Int_t obsValue = 0) : RooUnitTest(TString::Format("Profile Likelihood Calculator - Poisson - Observed value: %d", obsValue), refFile, writeRef, verbose), fObsValue(obsValue) {
      if (obsValue < 0) {
         fObsValue = 0;
         Warning("TestBasic103", "Negative observed value %d has been passed for Poisson distribution.\n   Using default observed value (0) instead.", obsValue);
      }
   };

   Bool_t testCode() {
      TString lowerLimitString = TString::Format("rs103_lower_limit_mean_%d", fObsValue);
      TString upperLimitString = TString::Format("rs103_upper_limit_mean_%d", fObsValue);

      // Put the significance level so that we obtain a 68% confidence interval
      const Double_t testSize = ROOT::Math::normal_cdf_c(1) * 2; // significance level

      // Create Poisson model and dataset
      RooWorkspace* w = new RooWorkspace("w", kTRUE);
      w->factory("Poisson::poiss(x[0,1000], mean[0,1000])");
      // NOTE: true mean of Poisson distribution does not really matter in this case
      //       LL ratio method result depends solely on the observed value

      RooRealVar *x = w->var("x");
      x->setVal(fObsValue);
      RooArgSet *obsSet =  new RooArgSet(*x);
      RooDataSet *data = new RooDataSet("data", "data", *obsSet);
      data->add(*obsSet);

      if (_write == kTRUE) {

         // Solutions of equation 2*[ln(LL(xMax)) - ln(LL(x))] = 1, where xMax is the point of maximum likelihood
         // For the special case of the Poisson distribution with N = 1, xMax = obsValue
         TString llRatioExpression = TString::Format("2*(x-%d*log(x)-%d+%d*log(%d))", fObsValue, fObsValue, fObsValue, fObsValue);
         // Special case fObsValue = 0 because log(0) not computable, the limit of n * log(n), n->0 must be taken
         if (fObsValue == 0) llRatioExpression = TString::Format("2*x");


         TF1 *llRatio = new TF1("llRatio", llRatioExpression, 1e-100, fObsValue); // lowerLimit < obsValue
         Double_t lowerLimit = llRatio->GetX(1);
         llRatio->SetRange(fObsValue, 1000); // upperLimit > obsValue
         Double_t upperLimit = llRatio->GetX(1);

         // Compare the limits obtained via ProfileLikelihoodCalculator with the likelihood ratio analytic computations
         regValue(lowerLimit, lowerLimitString);
         regValue(upperLimit, upperLimitString);

         // Cleanup branch objects
         delete llRatio;

      } else {

         // Calculate likelihood interval using the ProfileLikelihoodCalculator
         RooArgSet *params = new RooArgSet();
         params->add(*w->var("mean"));
         ProfileLikelihoodCalculator *plc = new ProfileLikelihoodCalculator(*data, *w->pdf("poiss"), *params);
         plc->SetConfidenceLevel(1 - testSize);
         LikelihoodInterval *interval = plc->GetInterval();

         // Register externally computed limits in the reference file
         regValue(interval->LowerLimit(*w->var("mean")), lowerLimitString);
         regValue(interval->UpperLimit(*w->var("mean")), upperLimitString);

         // Cleanup branch objects
         delete params;
         delete plc;
         delete interval;
      }

      // Cleanup function objects
      delete obsSet;
      delete data;
      delete w;

      return kTRUE ;
   }

private:
   Int_t fObsValue;

} ;



///////////////////////////////////////////////////////////////////////////////
//
// BAYESIAN CENTRAL INTERVAL - SIMPLE MODEL
//
// Test the Bayesian central interval computed by the BayesianCalculator on a
// Poisson distribution, using different priors. The parameter of interest is
// the mean of the Poisson distribution, and there are no nuisance parameters.
// The priors used are:
//    1. constant / uniform
//    2. inverse of the mean
//    3. square root of the inverse of the mean
//    4. gamma distribution
// The posterior distribution are easily obtained analytically for these cases.
// Therefore, the reference interval limits will be computed analytically.
//
// 04/2012 - Ioan Gabriel Bucur
//
///////////////////////////////////////////////////////////////////////////////

#include "RooStats/BayesianCalculator.h"

class TestBayesianCalculator1 : public RooUnitTest {
public:
   TestBayesianCalculator1(TFile* refFile, Bool_t writeRef, Int_t verbose) : RooUnitTest("BayesianCalculator Central Interval - Poisson Simple Model", refFile, writeRef, verbose) {};

   Bool_t testCode() {

      // Put the significance level so that we obtain a 68% CL central interval
      const Double_t testSize = ROOT::Math::normal_cdf_c(1) * 2; // size of the test
      const Int_t obsValue = 3; // observed experiment value
      const Double_t gammaShape = 2; // shape of the gamma distribution prior (testSize)
      const Double_t gammaRate = 1; // rate = 1/scale of the gamma distribution prior (beta = 1/theta)


      if (_write == kTRUE) {

         Double_t lowerLimit = ROOT::Math::gamma_quantile(testSize / 2, obsValue + 1, 1); // integrate to 16%
         Double_t upperLimit = ROOT::Math::gamma_quantile_c(testSize / 2, obsValue + 1, 1); // integrate to 84%
         Double_t lowerLimitInv = ROOT::Math::gamma_quantile(testSize / 2, obsValue, 1); // integrate to 16%
         Double_t upperLimitInv = ROOT::Math::gamma_quantile_c(testSize / 2, obsValue, 1); // integrate to 84%
         Double_t lowerLimitInvSqrt = ROOT::Math::gamma_quantile(testSize / 2, obsValue + 0.5, 1); // integrate to 16%
         Double_t upperLimitInvSqrt = ROOT::Math::gamma_quantile_c(testSize / 2, obsValue + 0.5, 1); // integrate to 84%
         Double_t lowerLimitGamma = ROOT::Math::gamma_quantile(testSize / 2, obsValue + gammaShape, 1.0 / (1 + gammaRate)); // integrate to 16%
         Double_t upperLimitGamma = ROOT::Math::gamma_quantile_c(testSize / 2, obsValue + gammaShape, 1.0 / (1 + gammaRate)); // integrate to 84%

         // Compare the limits obtained via BayesianCalculator with quantile values
         regValue(lowerLimit, "bc1_lower_limit_unif");
         regValue(upperLimit, "bc1_upper_limit_unif");
         regValue(lowerLimitInv, "bc1_lower_limit_inv");
         regValue(upperLimitInv, "bc1_upper_limit_inv");
         regValue(lowerLimitInvSqrt, "bc1_lower_limit_inv_sqrt");
         regValue(upperLimitInvSqrt, "bc1_upper_limit_inv_sqrt");
         regValue(lowerLimitGamma, "bc1_lower_limit_gamma");
         regValue(upperLimitGamma, "bc1_upper_limit_gamma");

      } else {
         // Create Poisson model and dataset
         RooWorkspace* w = new RooWorkspace("w", kTRUE);
         // NOTE: Solve for boundary intervals
         w->factory("Poisson::poiss(x[0,100], mean[1e-6,30])");
         // 20 -> 26.6 sec; 33 -> 44 sec; 100 -> 68.7 sec
         w->factory("Uniform::prior(mean)");
         w->factory("CEXPR::priorInv('1/mean', mean)");
         w->factory("CEXPR::priorInvSqrt('1/sqrt(mean)', mean)");
         w->factory(TString::Format("Gamma::priorGamma(mean, %lf, %lf, 0)", gammaShape, gammaRate));

         //RooAbsReal::defaultIntegratorConfig()->getConfigSection("RooIntegrator1D").setRealValue("maxSteps", 30);
         RooAbsReal::defaultIntegratorConfig()->method1D().setLabel("RooAdaptiveGaussKronrodIntegrator1D");

         // NOTE: mean cannot actually be in the interval [0, 100]  due to log evaluation errors in BayesianCalculator
         RooRealVar *x = w->var("x");
         x->setVal(obsValue);
         RooArgSet *obsSet =  new RooArgSet(*x);
         RooDataSet *data = new RooDataSet("data", "data", *obsSet);
         data->add(*obsSet);

         // Calculate likelihood interval using the BayesianCalculator
         RooArgSet *params = new RooArgSet();
         params->add(*w->var("mean"));

         // Uniform prior on mean
         BayesianCalculator *bc = new BayesianCalculator(*data, *w->pdf("poiss"), *params, *w->pdf("prior"), NULL);
         bc->SetTestSize(testSize);
         SimpleInterval *interval = bc->GetInterval();
         regValue(interval->LowerLimit(), "bc1_lower_limit_unif");
         regValue(interval->UpperLimit(), "bc1_upper_limit_unif");

         delete bc;
         delete interval;

         // Inverse of mean prior
         bc = new BayesianCalculator(*data, *w->pdf("poiss"), *params, *w->pdf("priorInv"), NULL);
         bc->SetTestSize(testSize);
         interval = bc->GetInterval();
         regValue(interval->LowerLimit(), "bc1_lower_limit_inv");
         regValue(interval->UpperLimit(), "bc1_upper_limit_inv");

         delete bc;
         delete interval;

         // Square root of inverse of mean prior
         bc = new BayesianCalculator(*data, *w->pdf("poiss"), *params, *w->pdf("priorInvSqrt"), NULL);
         bc->SetTestSize(testSize);
         interval = bc->GetInterval();
         regValue(interval->LowerLimit(), "bc1_lower_limit_inv_sqrt");
         regValue(interval->UpperLimit(), "bc1_upper_limit_inv_sqrt");
         delete bc;
         delete interval;

         // Gamma distribution prior
         bc = new BayesianCalculator(*data, *w->pdf("poiss"), *params, *w->pdf("priorGamma"), NULL);
         bc->SetTestSize(testSize);
         interval = bc->GetInterval();
         regValue(interval->LowerLimit(), "bc1_lower_limit_gamma");
         regValue(interval->UpperLimit(), "bc1_upper_limit_gamma");

         // Cleanup branch objects
         delete bc;
         delete interval;
         delete params;
         delete obsSet;
         delete data;
         delete w;
      }

      return kTRUE ;
   }

};



///////////////////////////////////////////////////////////////////////////////
//
// BAYESIAN SHORTEST INTERVAL - SIMPLE POISSON MODEL
//
// Test the Bayesian shortest interval computed by the BayesianCalculator on a
// Poisson distribution, using different priors. The parameter of interest is
// the mean of the Poisson distribution, and there are no nuisance parameters.
// The priors used are:
//    1. constant / uniform
//    2. inverse of the mean
// The reference interval limits are taken from the paper: "Why isn't every
// physicist a Bayesian?" by Robert D. Cousins.
//
// 04/2012 - Ioan Gabriel Bucur
//
///////////////////////////////////////////////////////////////////////////////

class TestBayesianCalculator2 : public RooUnitTest {
public:
   TestBayesianCalculator2(TFile* refFile, Bool_t writeRef, Int_t verbose) : RooUnitTest("BayesianCalculator Shortest Interval - Poisson Simple Model", refFile, writeRef, verbose) {};

   Double_t vtol() {
      return 1e-2;   // the references values in the paper have a precision of only two decimal points
   }
   // in such a situation, it is natural that we increase the value tolerance

   Bool_t testCode() {

      // Put the significance level so that we obtain a 68% confidence interval
      const Double_t testSize = ROOT::Math::normal_cdf_c(1) * 2; // test size
      const Int_t obsValue = 3; // observed experiment value

      if (_write == kTRUE) {

         // Compare the limits obtained via BayesianCalculator with given reference values
         regValue(1.55, "bc2_lower_limit_unif");
         regValue(5.15, "bc2_upper_limit_unif");
         regValue(0.86, "bc2_lower_limit_inv");
         regValue(3.85, "bc2_upper_limit_inv");

      } else {
         // Create Poisson model and dataset
         RooWorkspace* w = new RooWorkspace("w", kTRUE);
         // NOTE: Solve for boundary intervals
         w->factory("Poisson::poiss(x[0,100], mean[1e-6,100])");
         w->factory("Uniform::prior(mean)");
         w->factory("CEXPR::priorInv('1/mean', mean)");

         RooAbsReal::defaultIntegratorConfig()->method1D().setLabel("RooAdaptiveGaussKronrodIntegrator1D");

         // NOTE: mean cannot actually be in the interval [0, 100]  due to log evaluation errors in BayesianCalculator
         RooRealVar *x = w->var("x");
         x->setVal(obsValue);
         RooArgSet *obsSet =  new RooArgSet(*x);
         RooDataSet *data = new RooDataSet("data", "data", *obsSet);
         data->add(*obsSet);

         // Calculate likelihood interval using the BayesianCalculator
         RooArgSet *params = new RooArgSet();
         params->add(*w->var("mean"));

         // Uniform prior on mean
         BayesianCalculator *bc = new BayesianCalculator(*data, *w->pdf("poiss"), *params, *w->pdf("prior"), NULL);
         bc->SetTestSize(testSize);
         bc->SetShortestInterval();
         bc->SetScanOfPosterior(10000);
         SimpleInterval *interval = bc->GetInterval();
         regValue(interval->LowerLimit(), "bc2_lower_limit_unif");
         regValue(interval->UpperLimit(), "bc2_upper_limit_unif");

         delete bc;
         delete interval;

         // Inverse of mean prior
         bc = new BayesianCalculator(*data, *w->pdf("poiss"), *params, *w->pdf("priorInv"), NULL);
         bc->SetTestSize(testSize);
         bc->SetShortestInterval();
         bc->SetScanOfPosterior(10000);
         interval = bc->GetInterval();
         regValue(interval->LowerLimit(), "bc2_lower_limit_inv");
         regValue(interval->UpperLimit(), "bc2_upper_limit_inv");

         // Cleanup branch objects
         delete bc;
         delete interval;
         delete params;
         delete obsSet;
         delete data;
         delete w;
      }

      return kTRUE ;
   }

};





//-----------------------------------------------------------------------------
// The next tests use the same model returned by createComplexModel

static ModelConfig *createComplexModel(RooWorkspace *w)
{

   // Build model
   w->factory("Poisson::poiss1(x[0,40], sum::splusb1(sig1[1e-6,20], bkg1[0,20]))");
   w->factory("prod::sig2(2,sig1)");
   w->factory("Poisson::poiss2(y[0,40], sum::splusb2(sig2, bkg2[0,20]))");
   w->factory("Gamma::constr1(bkg1, 3, 1, 0)");
   w->factory("Gamma::constr2(bkg2, 4, 1, 0)");
   w->factory("PROD::model(poiss1, constr1, poiss2, constr2)");
   w->factory("Uniform::prior(sig1)");

   // build argument sets
   RooArgSet *obsSet =  new RooArgSet(*w->var("x"), *w->var("y"));
   RooArgSet *POISet = new RooArgSet(*w->var("sig1"));
   RooArgSet *NPSet = new RooArgSet(*w->var("bkg1"), *w->var("bkg2"));

   // build data set and import it into the workspace sets
   w->var("x")->setVal(7);
   w->var("y")->setVal(15);
   RooDataSet *data = new RooDataSet("data", "data", *obsSet);
   data->add(*obsSet);
   w->import(*data);

   // create model configuration
   ModelConfig *mc = new ModelConfig("ModelConfig", w);
   mc->SetObservables(*obsSet);
   mc->SetParametersOfInterest(*POISet);
   mc->SetNuisanceParameters(*NPSet);
   mc->SetPdf("model");
   mc->SetPriorPdf("prior");
   mc->SetSnapshot(*POISet);

   return mc;
}


///////////////////////////////////////////////////////////////////////////////
//
// BAYESIAN CENTRAL INTERVAL - COMPLEX POISSON MODEL
//
// Test the confidence interval computed by the BayesianCalculator on a
// complex model. A uniform prior is used for the POI.
//
// 04/2012 - Ioan Gabriel Bucur
//
///////////////////////////////////////////////////////////////////////////////

class TestBayesianCalculator3 : public RooUnitTest {

public:
   TestBayesianCalculator3(TFile* refFile, Bool_t writeRef, Int_t verbose) : RooUnitTest("BayesianCalculator - Poisson Complex Product Model", refFile, writeRef, verbose) {};

   Bool_t testCode() {

      const Double_t testSize = ROOT::Math::normal_cdf_c(1) * 2; // test size
      // for the BayesianCalculator we need to change the integration method - default method is too slow
      RooAbsReal::defaultIntegratorConfig()->method1D().setLabel("RooAdaptiveGaussKronrodIntegrator1D");

      // Create workspace model
      RooWorkspace *w = new RooWorkspace("w", kTRUE);
      ModelConfig *mc = createComplexModel(w);

      // Create BayesianCalculator and calculate interval
      BayesianCalculator *bc = new BayesianCalculator(*w->data("data"), *mc);
      bc->SetTestSize(testSize);
      SimpleInterval *interval = bc->GetInterval();
      regValue(interval->LowerLimit(), "bc3_lower_limit_sig1_unif");
      regValue(interval->UpperLimit(), "bc3_upper_limit_sig1_unif");

      // Obtain CI by scanning the posterior function in the given number of points
      bc->SetScanOfPosterior(100);
      interval = bc->GetInterval();
      regValue(interval->LowerLimit(), "bc3_lower_limit_sig1_binned");
      regValue(interval->UpperLimit(), "bc3_upper_limit_sig1_binned");

      // Cleanup
      delete bc;
      delete interval;
      delete w;

      return kTRUE ;
   }
};



///////////////////////////////////////////////////////////////////////////////
//
// MCMC INTERVAL CALCULATOR - COMPLEX MODEL
//
// Test the confidence interval computed by the MCMCCalculator on a complex
// model. A uniform prior is used for the POI.
//
// 04/2012 - Ioan Gabriel Bucur
//
///////////////////////////////////////////////////////////////////////////////

#include "RooStats/MCMCCalculator.h"
#include "RooStats/SequentialProposal.h"

class TestMCMCCalculator : public RooUnitTest {
public:
   TestMCMCCalculator(TFile* refFile, Bool_t writeRef, Int_t verbose) : RooUnitTest("MCMCCalculator - Poisson Complex Model", refFile, writeRef, verbose) {};

   Bool_t testCode() {

      const Double_t testSize = ROOT::Math::normal_cdf_c(1) * 2;

      // Create workspace and model
      RooWorkspace *w = new RooWorkspace("w", kTRUE);
      ModelConfig *mc = createComplexModel(w);

      // build confidence interval (MCMCInterval) with MCMC calculator
      SequentialProposal *sp = new SequentialProposal(0.1);
      MCMCCalculator *mcmcc = new MCMCCalculator(*w->data("data"), *mc);
      mcmcc->SetProposalFunction(*sp);
      mcmcc->SetNumIters(1000000); // Metropolis-Hastings algorithm iterations
      mcmcc->SetNumBurnInSteps(50); // first 50 steps to be ignored as burn-in
      mcmcc->SetTestSize(testSize);

      MCMCInterval *interval = mcmcc->GetInterval();
      regValue(interval->LowerLimit(*w->var("sig1")), "mcmcc_lower_limit_sig1");
      regValue(interval->UpperLimit(*w->var("sig1")), "mcmcc_upper_limit_sig1");

      // cleanup
      delete interval;
      delete mcmcc;
      delete sp;
      delete w;

      return kTRUE ;
   }
};



///////////////////////////////////////////////////////////////////////////////
//
// PROFILE LIKELIHOOD INTERVAL CALCULATOR - COMPLEX MODEL
//
// Test the likelihood interval computed by the ProfileLikelihoodCalculator
// on a complex model. A uniform prior is used for the POI.
//
// 04/2012 - Ioan Gabriel Bucur
//
///////////////////////////////////////////////////////////////////////////////

class TestProfileLikelihoodCalculator2 : public RooUnitTest {
public:
   TestProfileLikelihoodCalculator2(TFile* refFile, Bool_t writeRef, Int_t verbose) : RooUnitTest("ProfileLikelihoodCalculator - Poisson Complex Model", refFile, writeRef, verbose) {};

   Bool_t testCode() {

      const Double_t testSize = ROOT::Math::normal_cdf_c(1) * 2;

      // Create workspace and model
      RooWorkspace *w = new RooWorkspace("w", kTRUE);
      ModelConfig *mc = createComplexModel(w);

      // build likelihood interval with ProfileLikelihoodCalculator
      ProfileLikelihoodCalculator *plc = new ProfileLikelihoodCalculator(*w->data("data"), *mc, testSize);
      LikelihoodInterval *interval = plc->GetInterval();
      regValue(interval->LowerLimit(*w->var("sig1")), "plc2_lower_limit_sig1");
      regValue(interval->UpperLimit(*w->var("sig1")), "plc2_upper_limit_sig1");

      // cleanup
      delete interval;
      delete plc;
      delete w;

      return kTRUE ;
   }
};



///////////////////////////////////////////////////////////////////////////////
//
// HYPOTESTINVERTER INTERVAL - COMPLEX MODEL
//
// Test the likelihood interval computed by the ProfileLikelihoodCalculator
// on a complex model. A uniform prior is used for the POI.
//
// 04/2012 - Ioan Gabriel Bucur
//
///////////////////////////////////////////////////////////////////////////////

#include "RooStats/HypoTestInverter.h"
#include "RooStats/HypoTestInverterResult.h"
#include "RooStats/HypoTestCalculatorGeneric.h"
#include "RooStats/ToyMCSampler.h"
#include "RooStats/HybridCalculator.h"

#include "RooStats/HypoTestInverterPlot.h"
#include "TCanvas.h"
#include "RooStats/SamplingDistPlot.h"

class TestHypoTestInverter : public RooUnitTest {
private:
   HypoTestInverter::ECalculatorType fCalculatorType;

public:
   TestHypoTestInverter(TFile* refFile, Bool_t writeRef, Int_t verbose, HypoTestInverter::ECalculatorType calculatorType) : RooUnitTest(TString::Format("HypoTestInverter - Poisson Complex Model - Calculator Type %d", calculatorType), refFile, writeRef, verbose) {
      fCalculatorType = calculatorType;  
   };

   Bool_t testCode() {

      RooMsgService::instance().setGlobalKillBelow(RooFit::FATAL);

      const Double_t testSize = ROOT::Math::normal_cdf_c(1) * 2;

      // Create workspace and model
      RooWorkspace *w = new RooWorkspace("w", kTRUE);
      ModelConfig *mc = createComplexModel(w);

      // Background model -> sее createComplexModel for variable names
      w->factory("Poisson::poiss1bkg(x, bkg1)");
      w->factory("Poisson::poiss2bkg(y, bkg2)");
      w->factory("PROD::modelbkg(poiss1bkg, constr1, poiss2bkg, constr2)");
      ModelConfig *mcbkg = new ModelConfig(*mc);
      mcbkg->SetName("ModelConfigBackground");
      mcbkg->SetPdf("modelbkg");
      ((RooRealVar *)mcbkg->GetParametersOfInterest()->first())->setVal(1e-06);
      mcbkg->SetSnapshot(*mcbkg->GetParametersOfInterest()->first());

      // build confidence interval with HypoTestInverter
      HypoTestInverter *hti = new HypoTestInverter(*w->data("data"), *mc, *mcbkg, NULL, fCalculatorType, testSize);
      hti->SetFixedScan(5, 2, 8);
      
      if(fCalculatorType == HypoTestInverter::kHybrid) {
         // force prior nuisance pdf
         HybridCalculator *hc = (HybridCalculator *)hti->GetHypoTestCalculator();
         w->factory("PROD::priorbkg(constr1, constr2)");
         cout << "Prior background exists " << w->pdf("priorbkg") << endl;
         hc->ForcePriorNuisanceNull(*w->pdf("priorbkg"));
         hc->ForcePriorNuisanceAlt(*w->pdf("priorbkg"));
      }

      // needed at this moment, because we have no extended pdf and the ToyMC Sampler evaluation returns an errori
      ToyMCSampler *tmcs = (ToyMCSampler *)hti->GetHypoTestCalculator()->GetTestStatSampler();
      tmcs->SetMaxToys(300);
      tmcs->SetNEventsPerToy(1);
      HypoTestInverterResult *interval = hti->GetInterval();
      regValue(interval->LowerLimit(), TString::Format("hti_lower_limit_sig1_calc_%d", fCalculatorType));
      regValue(interval->UpperLimit(), TString::Format("hti_upper_limit_sig1_calc_%d", fCalculatorType));

      if(_verb >= 1) {
         cout << "[" << interval->LowerLimit() << "," << interval->UpperLimit() << "]" << endl;
         HypoTestInverterPlot *plot = new HypoTestInverterPlot("HTI_Result_Plot", "Feldman-Cousins Interval", interval);
         TCanvas *c1 = new TCanvas("HypoTestInverter Scan");
         c1->SetLogy(false);
         plot->Draw("OBS");
         c1->SaveAs("HypoTestInverterScan.pdf");

         if(_verb == 2) {
            const int n = interval->ArraySize();
            if (n > 0 && interval->GetResult(0)->GetNullDistribution()) {
               TCanvas *c2 = new TCanvas("Test Statistic Distributions", "", 2);
               if (n > 1) {
                  int ny = TMath::CeilNint(sqrt(n));
                  int nx = TMath::CeilNint(double(n) / ny);
                  c2->Divide(nx, ny);
               }
               for (int i = 0; i < n; i++) {
                  if (n > 1) c2->cd(i + 1);
                  SamplingDistPlot *pl = plot->MakeTestStatPlot(i);
                  pl->SetLogYaxis(true);
                  pl->Draw();
               }
               c2->SaveAs("HypoTestInverterTestStatDistributions.pdf");
            }
         }
      }


      // cleanup
      delete interval;
      delete hti;
      delete w;

      return kTRUE ;
   }
};


