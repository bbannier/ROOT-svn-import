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
// normal_quantile_c(alpha/2, 1). In the case of a Gaussian distribution, the 
// interval limits are equal to: mean +- normal_quantile_c(alpha/2, sigma/sqrt(N)).
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

      const Double_t alpha = 0.05; // significance level
      const Int_t N = 1; // number of observations

      //TODO: see why it fails for a small number of observations

      // Create Gaussian model and dataset
      RooWorkspace* w = new RooWorkspace("w", kTRUE);
      w->factory("Gaussian::gauss(x[-5,5], mean[0,-5,5], sigma[1])");
      RooDataSet *data = w->pdf("gauss")->generate(*w->var("x"), N);


      if (_write == kTRUE) {

         // Calculate likelihood interval from data via analytic methods
         Double_t estMean = data->mean(*w->var("x"));
         Double_t intervalHalfWidth = ROOT::Math::normal_quantile_c(alpha / 2.0, w->var("sigma")->getValV() / sqrt(N));
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
         plc->SetTestSize(alpha);

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
      if(obsValue < 0) {
         fObsValue = 0;
         Warning( "TestBasic103", "Negative observed value %d has been passed for Poisson distribution.\n   Using default observed value (0) instead.", obsValue); 
      }
   };

    Bool_t testCode() {
      TString lowerLimitString = TString::Format("rs103_lower_limit_mean_%d", fObsValue);
      TString upperLimitString = TString::Format("rs103_upper_limit_mean_%d", fObsValue);

      // Put the significance level so that we obtain a 68% confidence interval
      const Double_t alpha = ROOT::Math::normal_cdf_c(1) * 2; // significance level

      // Create Poisson model and dataset
      RooWorkspace* w = new RooWorkspace("w", kTRUE);
      w->factory("Poisson::poiss(x[0,1000], mean[0,1000])"); 
      // NOTE: true mean of Poisson distribution does not really matter in this case
      //       LL ratio method result depends solely on the observed value

      RooRealVar *x = w->var("x");
      x->setVal(fObsValue);
      RooArgSet *obsSet =  new RooArgSet(*x);
      RooDataSet *data = new RooDataSet("poissData", "Poisson distribution data", *obsSet);
      data->add(*obsSet);

      if (_write == kTRUE) {

         // Solutions of equation 2*[ln(LL(xMax)) - ln(LL(x))] = 1, where xMax is the point of maximum likelihood
         // For the special case of the Poisson distribution with N = 1, xMax = obsValue
         TString llRatioExpression = TString::Format("2*(x-%d*log(x)-%d+%d*log(%d))", fObsValue, fObsValue, fObsValue, fObsValue);
         // Special case fObsValue = 0 because log(0) not computable, the limit of n * log(n), n->0 must be taken
         if(fObsValue == 0) llRatioExpression = TString::Format("2*x");       


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
         plc->SetConfidenceLevel(1 - alpha);
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
// 'BAYESIAN CENTRAL INTERVAL - SIMPLE MODEL' RooStats Tutorial Macro #104
//
// Test the central interval computed by the Bayesian calculator on a Poisson 
// distribution, using different priors. The parameter of interest is the mean of
// the Poisson distribution, and there are no nuisance parameters. The priors used are:
//    1. constant / uniform 
//    2. inverse of the mean
//    3. square root of the inverse of the mean
//    4. gamma distribution
//
// 04/2012 - Ioan Gabriel Bucur
//
///////////////////////////////////////////////////////////////////////////////

#include "RooGaussian.h"
#include "RooPoisson.h" 
#include "RooStats/BayesianCalculator.h"

class TestBasic104 : public RooUnitTest {
public:
   TestBasic104(TFile* refFile, Bool_t writeRef, Int_t verbose) : RooUnitTest("Bayesian Calculator vs Analytical - Poisson Simple Model Central Interval", refFile, writeRef, verbose) {}; 

   Bool_t testCode() {

            // Put the significance level so that we obtain a 68% confidence interval
      const Double_t alpha = ROOT::Math::normal_cdf_c(1) * 2; // significance level
      const Int_t obsValue = 3; // observed experiment value
      const Double_t gammaShape = 2; // shape of the gamma distribution prior (alpha)
      const Double_t gammaRate = 1; // rate = 1/scale of the gamma distribution prior (beta = 1/theta)

      if (_write == kTRUE) {

         Double_t lowerLimit = ROOT::Math::gamma_quantile(alpha / 2, obsValue + 1, 1); // integrate to 16%
         Double_t upperLimit = ROOT::Math::gamma_quantile_c(alpha / 2, obsValue + 1, 1); // integrate to 84%
         Double_t lowerLimitInv = ROOT::Math::gamma_quantile(alpha / 2, obsValue, 1); // integrate to 16%
         Double_t upperLimitInv = ROOT::Math::gamma_quantile_c(alpha / 2, obsValue, 1); // integrate to 84%
         Double_t lowerLimitInvSqrt = ROOT::Math::gamma_quantile(alpha / 2, obsValue + 0.5, 1); // integrate to 16%
         Double_t upperLimitInvSqrt = ROOT::Math::gamma_quantile_c(alpha / 2, obsValue + 0.5, 1); // integrate to 84%
         Double_t lowerLimitGamma = ROOT::Math::gamma_quantile(alpha / 2, obsValue + gammaShape, 1.0/(1 + gammaRate)); // integrate to 16%
         Double_t upperLimitGamma = ROOT::Math::gamma_quantile_c(alpha / 2, obsValue + gammaShape, 1.0/(1 + gammaRate)); // integrate to 84%

         // Compare the limits obtained via BayesianCalculator with quantile values 
         regValue(lowerLimit, "rs104_lower_limit_unif");
         regValue(upperLimit, "rs104_upper_limit_unif");
         regValue(lowerLimitInv, "rs104_lower_limit_inv");
         regValue(upperLimitInv, "rs104_upper_limit_inv");
         regValue(lowerLimitInvSqrt, "rs104_lower_limit_inv_sqrt");
         regValue(upperLimitInvSqrt, "rs104_upper_limit_inv_sqrt");
         regValue(lowerLimitGamma, "rs104_lower_limit_gamma");
         regValue(upperLimitGamma, "rs104_upper_limit_gamma");         
         
      } else {
         // Create Poisson model and dataset
         RooWorkspace* w = new RooWorkspace("w", kTRUE);
         // NOTE: Solve for boundary intervals
         w->factory("Poisson::poiss(x[0,100], mean[1e-6,100])");
         w->factory("Uniform::prior(mean)");
         w->factory("CEXPR::priorInv('1/mean', mean)");
         w->factory("CEXPR::priorInvSqrt('1/sqrt(mean)', mean)");
         w->factory(TString::Format("Gamma::priorGamma(mean, %lf, %lf, 0)", gammaShape, gammaRate));

         RooAbsReal::defaultIntegratorConfig()->method1D().setLabel("RooAdaptiveGaussKronrodIntegrator1D");

         // NOTE: mean cannot actually be in the interval [0, 100]  due to log evaluation errors in BayesianCalculator
         RooRealVar *x = w->var("x");
         x->setVal(obsValue);
         RooArgSet *obsSet =  new RooArgSet(*x);
         RooDataSet *data = new RooDataSet("poissData", "Poisson distribution data", *obsSet);
         data->add(*obsSet);

         // Calculate likelihood interval using the BayesianCalculator 
         RooArgSet *params = new RooArgSet();
         params->add(*w->var("mean"));

         // Uniform prior on mean
         BayesianCalculator *bc = new BayesianCalculator(*data, *w->pdf("poiss"), *params, *w->pdf("prior"), NULL);
         bc->SetConfidenceLevel(1 - alpha);
         SimpleInterval *interval = bc->GetInterval();         
         regValue(interval->LowerLimit(), "rs104_lower_limit_unif");
         regValue(interval->UpperLimit(), "rs104_upper_limit_unif");
     
         cout << interval->LowerLimit() << " " << interval->UpperLimit()<<endl;
 
         delete bc;
         delete interval;

         // Inverse of mean prior           
         bc = new BayesianCalculator(*data, *w->pdf("poiss"), *params, *w->pdf("priorInv"), NULL);
         bc->SetConfidenceLevel(1 - alpha);
         interval = bc->GetInterval();         
         regValue(interval->LowerLimit(), "rs104_lower_limit_inv");
         regValue(interval->UpperLimit(), "rs104_upper_limit_inv");         
cout << interval->LowerLimit() << " " << interval->UpperLimit()<<endl;
 
         delete bc;
         delete interval;

         // Square root of inverse of mean prior           
         bc = new BayesianCalculator(*data, *w->pdf("poiss"), *params, *w->pdf("priorInvSqrt"), NULL);
         bc->SetConfidenceLevel(1 - alpha);
         interval = bc->GetInterval();         
         regValue(interval->LowerLimit(), "rs104_lower_limit_inv_sqrt");
         regValue(interval->UpperLimit(), "rs104_upper_limit_inv_sqrt");
 cout << interval->LowerLimit() << " " << interval->UpperLimit()<<endl;
         delete bc;
         delete interval;

         // Gamma distribution prior           
          bc = new BayesianCalculator(*data, *w->pdf("poiss"), *params, *w->pdf("priorGamma"), NULL);
          bc->SetConfidenceLevel(1 - alpha);
          interval = bc->GetInterval();         
          regValue(interval->LowerLimit(), "rs104_lower_limit_gamma");
          regValue(interval->UpperLimit(), "rs104_upper_limit_gamma");  
cout << interval->LowerLimit() << " " << interval->UpperLimit()<<endl;
 
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
// 'BAYESIAN INTERVAL CALCULATOR - COMPLEX MODEL' RooStats Tutorial Macro #105
//
// Test the confidence interval computed by the Bayesian calculator on a signal +
// background Poisson model, using 1. constant / uniform prior and 2. square root 
// of the inverse of the mean prior. 
//
// 04/2012 - Ioan Gabriel Bucur
//
///////////////////////////////////////////////////////////////////////////////

class TestBasic105 : public RooUnitTest {

public:
   TestBasic105(TFile* refFile, Bool_t writeRef, Int_t verbose, Int_t obsValue = 0) : RooUnitTest(TString::Format("Bayesian Calculator - Poisson with One Nuisance Parameter Central Interval- Observed Value %d", obsValue), refFile, writeRef, verbose), fObsValue(obsValue) {
      if(obsValue < 0) {
         fObsValue = 0;
         Warning( "TestBasic105", "Negative observed value %d has been passed for Poisson distribution.\n   Using default observed value (0) instead.", obsValue); 
      }
   }; 


   Bool_t testCode() {

      // We test two separate cases
      // In the first case, the prior distribution for the mean is simply a uniform distribution
      // In the second case, the prior distribution is uniform on the inverse of the mean
      TString lowerLimitString = TString::Format("rs105_lower_limit_mean_%d", fObsValue);
      TString upperLimitString = TString::Format("rs105_upper_limit_mean_%d", fObsValue);
      TString lowerLimitInvSqrtString = TString::Format("rs105_lower_limit_mean_invsqrt_%d", fObsValue);
      TString upperLimitInvSqrtString = TString::Format("rs105_upper_limit_mean_invsqrt_%d", fObsValue); 

      // Put the significance level so that we obtain a 68% confidence interval
      const Double_t alpha = ROOT::Math::normal_cdf_c(1) * 2; // significance level

  
      // Create Poisson model and dataset
      RooWorkspace* w = new RooWorkspace("w", kTRUE); 
      // background is normally distributed with mean 10 and variance 1
      w->factory("Poisson::poiss(x[0,100], sum::splusb(signal[0,100], Gaussian::bg(y[0,20], 10, 5)))");
      w->factory("Uniform::prior(splusb)");
      w->factory("CEXPR::priorInvSqrt('1/sqrt(splusb)', splusb)");

      // TODO: make RooIntegrator1D work
      RooAbsReal::defaultIntegratorConfig()->method1D().setLabel("RooAdaptiveGaussKronrodIntegrator1D");

      // create data set and argument sets
      RooRealVar *x = w->var("x");
      x->setVal(fObsValue);
      RooArgSet *obsSet =  new RooArgSet(*x);
      RooDataSet *data = new RooDataSet("poissData", "Poisson distribution data", *obsSet);
      data->add(*obsSet);
      RooArgSet *POISet = new RooArgSet();
      POISet->add(*w->var("signal"));
      RooArgSet *NPSet = new RooArgSet();
      //NPSet->add(*w->var("bg"));


      // Uniform prior on mean
      BayesianCalculator *bc = new BayesianCalculator(*data, *w->pdf("poiss"), *POISet, *w->pdf("prior"), NPSet);
      bc->SetConfidenceLevel(1 - alpha);
      SimpleInterval *interval = bc->GetInterval();         
      regValue(interval->LowerLimit(), lowerLimitString);
      regValue(interval->UpperLimit(), upperLimitString);
         
      delete bc;
      delete interval;

      // Prior is inverse of square root            
      bc = new BayesianCalculator(*data, *w->pdf("poiss"), *POISet, *w->pdf("priorInvSqrt"), NPSet);
      bc->SetConfidenceLevel(1 - alpha);
      interval = bc->GetInterval();         
      regValue(interval->LowerLimit(), lowerLimitInvSqrtString);
      regValue(interval->UpperLimit(), upperLimitInvSqrtString);         

      // Cleanup
      delete bc;
      delete interval;
      delete POISet;
      delete NPSet;   
      delete obsSet;
      delete data;
      delete w;      
      
      return kTRUE ;
   }

private:
   Int_t fObsValue;

};



///////////////////////////////////////////////////////////////////////////////
//
// 'MCMC INTERVAL CALCULATOR - COMPLEX MODEL' RooStats Tutorial Macro #106
//
// Test the confidence interval computed by the MCMC calculator on a signal +
// background Poisson model, using 1. constant / uniform prior and 2. square root 
// of the inverse of the mean prior. 
//
// 04/2012 - Ioan Gabriel Bucur
//
///////////////////////////////////////////////////////////////////////////////

#include "RooStats/MCMCCalculator.h"
#include "RooStats/SequentialProposal.h"

class TestBasic106 : public RooUnitTest {
public:
   TestBasic106(TFile* refFile, Bool_t writeRef, Int_t verbose, Int_t obsValue = 0) : RooUnitTest(TString::Format("MCMC Calculator vs Analytical Integrator - Poisson - Observed value %d", obsValue), refFile, writeRef, verbose), fObsValue(obsValue) {
      if(obsValue < 0) {
         fObsValue = 0;
         Warning( "TestBasic106", "Negative observed value %d has been passed for Poisson distribution.\n   Using default observed value (0) instead.", obsValue); 
      }
   }; 

   Bool_t testCode() {

      // We test two separate cases
      // In the first case, there is now prior distribution for the Poisson mean
      // In the second case, the prior distribution is the sqrt of the inverse of the mean

      TString lowerLimitString = TString::Format("rs106_lower_limit_mean_%d", fObsValue);
      TString upperLimitString = TString::Format("rs106_upper_limit_mean_%d", fObsValue);
      TString lowerLimitInvSqrtString = TString::Format("rs106_lower_limit_mean_invsqrt_%d", fObsValue);
      TString upperLimitInvSqrtString = TString::Format("rs106_upper_limit_mean_invsqrt_%d", fObsValue);
 
      // Create Poisson model and dataset
      RooWorkspace* w = new RooWorkspace("w", kTRUE); 
      // background is normally distributed with mean 10 and variance 1
      w->factory("Poisson::poiss(x[0,100], sum::splusb(signal[0,100], Gaussian::bg(y[0,20], 10, 5)))");
      w->factory("CEXPR::priorInvSqrt('1/sqrt(splusb)', splusb)");

      // TODO: make RooIntegrator1D work
      // RooAbsReal::defaultIntegratorConfig()->method1D().setLabel("RooAdaptiveGaussKronrodIntegrator1D");

      // build data set and argument sets
      RooRealVar *x = w->var("x");
      x->setVal(fObsValue);
      RooArgSet *obsSet =  new RooArgSet(*x);
      RooDataSet *data = new RooDataSet("poissData", "Poisson distribution data", *obsSet);
      data->add(*obsSet);      
      RooArgSet *POISet = new RooArgSet();
      POISet->add(*w->var("signal"));
      RooArgSet *NPSet = new RooArgSet();
      //NPSet->add(*w->pdf("bg"));   
        
      // build confidence interval (MCMCInterval) with MCMC calculator (size 5%)
      SequentialProposal *sp = new SequentialProposal(0.1);
      MCMCCalculator *mcmcc = new MCMCCalculator();
      mcmcc->SetData(*data);
      mcmcc->SetPdf(*w->pdf("poiss"));
      mcmcc->SetParameters(*POISet);
      //mcmcc->SetNuisanceParameters(*NPSet);
      mcmcc->SetProposalFunction(*sp);
      mcmcc->SetNumIters(1000000); // Metropolis-Hastings algorithm iterations
      mcmcc->SetNumBurnInSteps(50); // first 50 steps to be ignored as burn-in
      mcmcc->SetTestSize(0.05);

      // no prior
      MCMCInterval *interval = mcmcc->GetInterval();   
      regValue(interval->LowerLimit(*w->var("signal")), lowerLimitString);
      regValue(interval->UpperLimit(*w->var("signal")), upperLimitString);         
      delete interval;

      // prior is sqrt of inverse
      mcmcc->SetPriorPdf(*w->pdf("priorInvSqrt"));
      interval = mcmcc->GetInterval();
      regValue(interval->LowerLimit(*w->var("signal")), lowerLimitInvSqrtString);
      regValue(interval->UpperLimit(*w->var("signal")), upperLimitInvSqrtString);

      // cleanup
      delete mcmcc;
      delete interval;
      delete obsSet;
      delete POISet;
      delete NPSet;
      delete data;
      delete sp;
      delete w;
      
      return kTRUE ;
   }

private:
   Int_t fObsValue;

};


