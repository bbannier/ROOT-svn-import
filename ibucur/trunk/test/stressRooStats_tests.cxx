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

// C/C++ Standard Libraries
#include <utility>

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
#include "RooStats/TestStatistic.h"

using namespace RooFit;
using namespace RooStats;
using namespace ROOT::Math;




// testStatType = 0 Simple Likelihood Ratio (the LEP TestStat)
//              = 1 Ratio of Profiled Likelihood Ratios (the Tevatron TestStat)
//              = 2 Profile Likelihood Ratio (the LHC TestStat)
//              = 3 Profile Likelihood One Sided (pll = 0 if mu < mu_hat)
//              = 4 Profile Likelihood Signed (pll = -pll if mu < mu_hat)
//              = 5 Max Likelihood Estimate as test statistic
//              = 6 Number of Observed Events as test statistic
enum ETestStatType { kSimpleLR = 0, kRatioLR = 1, kProfileLR= 2, kProfileLROneSided = 3, kProfileLRSigned = 4, kMLE = 5, kNObs = 6 };
static const char *kECalculatorTypeString[] = { "Undefined", "Frequentist", "Hybrid", "Asymptotic" };
static const char *kETestStatTypeString[] = { "Simple Likelihood Ratio", "Ratio Likelihood Ratio", "Profile Likelihood Ratio", "Profile Likelihood One Sided", "Profile Likelihood Signed", "Max Likelihood Estimate", "Number of Observed Events" };
static TestStatistic *buildTestStatistic(const ETestStatType testStatType, const ModelConfig &sbModel, const ModelConfig &bModel);




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
// 'HYPOTESTCALCULATOR'
//
// This test evaluetes the functionality of the HypoTestCalculators.
//
// 04/2012 - Ioan Gabriel Bucur
// Insightful comments courtesy of Kyle Cranmer, Wouter Verkerke, Sven Kreiss
//    from $ROOTSYS/tutorials/roostats/HybridInstructional.C
//
///////////////////////////////////////////////////////////////////////////////


#include "RooStats/ProfileLikelihoodTestStat.h"
#include "RooStats/SimpleLikelihoodRatioTestStat.h"
#include "RooStats/HypoTestCalculatorGeneric.h"
#include "RooStats/FrequentistCalculator.h"
#include "RooStats/HybridCalculator.h"
#include "RooStats/AsymptoticCalculator.h"
#include "RooStats/ProfileLikelihoodCalculator.h"
#include "RooStats/MaxLikelihoodEstimateTestStat.h"
#include "RooStats/NumEventsTestStat.h"
#include "RooStats/RatioOfProfiledLikelihoodsTestStat.h"


class TestHypoTestCalculator : public RooUnitTest {
public:
   TestHypoTestCalculator(TFile* refFile, Bool_t writeRef, Int_t verbose) : RooUnitTest("HypoTestCalculator - On / Off Problem", refFile, writeRef, verbose) {};

   Bool_t testCode() {

      const Int_t xValue = 150;
      const Int_t yValue = 100;
      const Double_t tauValue = 1;
      
      if(_write == kTRUE) {

         // register analytical Z_Bi value
         Double_t Z_Bi = NumberCountingUtils::BinomialWithTauObsZ(xValue, yValue, tauValue);
         regValue(Z_Bi, "thtc_significance_hybrid");

      } else {
      
         // Make model for prototype on/off problem
         // Pois(x | s+b) * Pois(y | tau b )
         RooWorkspace* w = new RooWorkspace("w", kTRUE);
         w->factory(TString::Format("Poisson::on_pdf(x[%d,0,500],sum::splusb(sig[0,0,100],bkg[100,0,300]))", xValue));
         w->factory(TString::Format("Poisson::off_pdf(y[%d,0,500],prod::taub(tau[%lf],bkg))", yValue, tauValue));
         w->factory("PROD::prod_pdf(on_pdf, off_pdf)");        

         w->var("x")->setVal(xValue);
         w->var("y")->setVal(yValue);
         w->var("y")->setConstant();
         w->var("tau")->setVal(tauValue); 

        // construct the Bayesian-averaged model (eg. a projection pdf)
         // p'(x|s) = \int db p(x|s+b) * [ p(y|b) * prior(b) ]
         w->factory("Uniform::prior(bkg)");
         w->factory("PROJ::averagedModel(PROD::foo(on_pdf|bkg,off_pdf,prior),bkg)") ;

         // define sets of variables obs={x} and poi={sig}
         // x is the only observable in the main measurement and y is treated as a separate measurement,
         // which is used to produce the prior that will be used in the calculation to randomize the nuisance parameters
         w->defineSet("obs", "x");
         w->defineSet("poi", "sig");

         // Add observable value to a data set
         RooDataSet *data = new RooDataSet("data", "data", *w->set("obs"));
         data->add(*w->set("obs"));


         // Build S+B and B models
         ModelConfig *sbModel = new ModelConfig("SB_ModelConfig", w);
         sbModel->SetPdf(*w->pdf("prod_pdf"));
         sbModel->SetObservables(*w->set("obs"));      
         sbModel->SetParametersOfInterest(*w->set("poi"));
         w->var("sig")->setVal(xValue - yValue / tauValue); // important !
         sbModel->SetSnapshot(*w->set("poi"));

         ModelConfig *bModel = new ModelConfig("B_ModelConfig", w);
         bModel->SetPdf(*w->pdf("prod_pdf"));
         bModel->SetObservables(*w->set("obs"));
         bModel->SetParametersOfInterest(*w->set("poi"));
         w->var("sig")->setVal(0.0); // important !
         bModel->SetSnapshot(*w->set("poi"));

         // alternate priors
         w->factory("Gaussian::gauss_prior(bkg, y, expr::sqrty('sqrt(y)', y))");
         w->factory("Lognormal::lognorm_prior(bkg, y, expr::kappa('1+1./sqrt(y)',y))");

         // build test statistic
         SimpleLikelihoodRatioTestStat *slrts =  new SimpleLikelihoodRatioTestStat(*bModel->GetPdf(), *sbModel->GetPdf());
         slrts->SetNullParameters(*bModel->GetSnapshot());
         slrts->SetAltParameters(*sbModel->GetSnapshot());
         slrts->SetAlwaysReuseNLL(kTRUE);

         RatioOfProfiledLikelihoodsTestStat *roplts = new RatioOfProfiledLikelihoodsTestStat(*bModel->GetPdf(), *sbModel->GetPdf());
         roplts->SetAlwaysReuseNLL(kTRUE);

         ProfileLikelihoodTestStat *pllts = new ProfileLikelihoodTestStat(*bModel->GetPdf());
         pllts->SetAlwaysReuseNLL(kTRUE);
         pllts->SetOneSidedDiscovery(kTRUE);

         MaxLikelihoodEstimateTestStat *mlets = 
            new MaxLikelihoodEstimateTestStat(*sbModel->GetPdf(), *((RooRealVar *)sbModel->GetParametersOfInterest()->first()));
      
         NumEventsTestStat *nevts = new NumEventsTestStat(*sbModel->GetPdf());

         // ProfileLikelihoodCalculator *plc = new ProfileLikelihoodCalculator(*data, *sbModel);
         // plc->SetNullParameters(*bModel->GetSnapshot());
         // HypoTestResult *htr0 = plc->GetHypoTest();
         // htr0->Print(); 



         HybridCalculator *htc = new HybridCalculator(*data, *sbModel, *bModel);
         ToyMCSampler *tmcs = (ToyMCSampler *)htc->GetTestStatSampler();
         tmcs->SetNEventsPerToy(1);
         htc->SetToys(5000, 1000);
         htc->ForcePriorNuisanceAlt(*w->pdf("off_pdf"));
         htc->ForcePriorNuisanceNull(*w->pdf("off_pdf"));

         tmcs->SetTestStatistic(pllts);
         HypoTestResult *htr = htc->GetHypoTest();
         htr->Print();
         cout << "PLLTS " << htr->Significance() << endl;
         tmcs->SetTestStatistic(mlets);
         htr = htc->GetHypoTest();
         htr->Print();
         cout << "MLETS " << htr->Significance() << endl;
         tmcs->SetTestStatistic(nevts);
         htr = htc->GetHypoTest();
         htr->Print();
         cout << "NEVTS " << htr->Significance() << endl;
         tmcs->SetTestStatistic(slrts);
         htr = htc->GetHypoTest();
         htr->Print();
         cout << "SLRTS " << htr->Significance() << endl;
         tmcs->SetTestStatistic(roplts);
         htr = htc->GetHypoTest();
         htr->Print();
         cout << "ROPLTS " << htr->Significance() << endl;
 

         regValue(htr->Significance(), "thtc_significance_hybrid");

         delete htc;
         delete htr;
         delete w;
         delete data;
      }

      return kTRUE ;
   }
} ;


class TestHypoTestCalculator2 : public RooUnitTest {
public:
   TestHypoTestCalculator2(TFile* refFile, Bool_t writeRef, Int_t verbose) : RooUnitTest("HypoTestCalculator Frequentist - On / Off Problem", refFile, writeRef, verbose) {};

   Bool_t testCode() {

      const Int_t xValue = 150;
      const Int_t yValue = 100;
      const Double_t tauValue = 1;
      
      if(_write == kTRUE) {

         // register analytical Z_Bi value
         Double_t Z_Bi = NumberCountingUtils::BinomialWithTauObsZ(xValue, yValue, tauValue);
         regValue(Z_Bi, "thtc_significance_frequentist");

      } else {
      
         // Make model for prototype on/off problem
         // Pois(x | s+b) * Pois(y | tau b )
         RooWorkspace* w = new RooWorkspace("w", kTRUE);
         w->factory("Poisson::on_pdf(x[150,0,500],sum::splusb(sig[0,0,100],bkg[100,0,300]))");
         w->factory("Poisson::off_pdf(y[100,0,500],prod::taub(tau[1],bkg))");
         w->factory("PROD::prod_pdf(on_pdf, off_pdf)");
        
         w->var("x")->setVal(xValue);
         w->var("y")->setVal(yValue);
         w->var("y")->setConstant();
         w->var("tau")->setVal(tauValue);
         w->var("tau")->setConstant();
         // construct the Bayesian-averaged model (eg. a projection pdf)
         // p'(x|s) = \int db p(x|s+b) * [ p(y|b) * prior(b) ]
         //w->factory("PROJ::averagedModel(PROD::foo(on_pdf|bkg,off_pdf,prior),bkg)") ;

         // define sets of variables obs={x} and poi={sig}
         // x is the only observable in the main measurement and y is treated as a separate measurement,
         // which is used to produce the prior that will be used in the calculation to randomize the nuisance parameters
         w->defineSet("obs", "x");
         w->defineSet("poi", "sig");
         w->defineSet("globObs", "y");

         // Add observable value to a data set
         RooDataSet *data = new RooDataSet("data", "data", *w->set("obs"));
         data->add(*w->set("obs"));

         // Build S+B and B models
         ModelConfig *sbModel = new ModelConfig("SB_ModelConfig", w);
         sbModel->SetPdf(*w->pdf("prod_pdf"));
         sbModel->SetObservables(*w->set("obs"));
         sbModel->SetGlobalObservables(*w->set("globObs"));      
         sbModel->SetParametersOfInterest(*w->set("poi"));
         w->var("sig")->setVal(xValue - yValue / tauValue); // important !
         sbModel->SetSnapshot(*w->set("poi"));

         ModelConfig *bModel = new ModelConfig("B_ModelConfig", w);
         bModel->SetPdf(*w->pdf("prod_pdf"));
         bModel->SetObservables(*w->set("obs"));
         bModel->SetGlobalObservables(*w->set("globObs"));
         bModel->SetParametersOfInterest(*w->set("poi")); 
         w->var("sig")->setVal(0.0); // important !
         bModel->SetSnapshot(*w->set("poi"));


         // alternate priors
         w->factory("Gaussian::gauss_prior(bkg, y, expr::sqrty('sqrt(y)', y))");
         w->factory("Lognormal::lognorm_prior(bkg, y, expr::kappa('1+1./sqrt(y)',y))");

         // build test statistic
         SimpleLikelihoodRatioTestStat *slrts =  new SimpleLikelihoodRatioTestStat(*bModel->GetPdf(), *sbModel->GetPdf());
         slrts->SetNullParameters(*bModel->GetSnapshot());
         slrts->SetAltParameters(*sbModel->GetSnapshot());
         slrts->SetAlwaysReuseNLL(kTRUE);

         RatioOfProfiledLikelihoodsTestStat *roplts = new RatioOfProfiledLikelihoodsTestStat(*bModel->GetPdf(), *sbModel->GetPdf());
         roplts->SetAlwaysReuseNLL(kTRUE);

         ProfileLikelihoodTestStat *pllts = new ProfileLikelihoodTestStat(*bModel->GetPdf());
         pllts->SetAlwaysReuseNLL(kTRUE);
         pllts->SetOneSidedDiscovery(kTRUE);

         MaxLikelihoodEstimateTestStat *mlets = 
            new MaxLikelihoodEstimateTestStat(*sbModel->GetPdf(), *((RooRealVar *)sbModel->GetParametersOfInterest()->first()));
      
         NumEventsTestStat *nevts = new NumEventsTestStat(*sbModel->GetPdf());

        /* ProfileLikelihoodCalculator *plc = new ProfileLikelihoodCalculator(*data, *sbModel);
         plc->SetNullParameters(*bModel->GetSnapshot());
         plc->SetAlternateParameters(*sbModel->GetSnapshot());
         HypoTestResult *htr0 = plc->GetHypoTest();
         cout << "PLC " << htr0->Significance() << endl; 
*/

         FrequentistCalculator *ftc = new FrequentistCalculator(*data, *sbModel, *bModel);
         ftc->SetToys(5000, 1000);
         ToyMCSampler *tmcs = (ToyMCSampler *)ftc->GetTestStatSampler();
         tmcs->SetNEventsPerToy(1); // because the model is in number counting form     
         tmcs->SetUseMultiGen(kTRUE);
         tmcs->SetAlwaysUseMultiGen(kTRUE);
         
         tmcs->SetTestStatistic(slrts);
         HypoTestResult *htr = ftc->GetHypoTest();
         htr->Print();
         cout << "SLRTS " << htr->Significance() << endl;
         tmcs->SetTestStatistic(pllts);
         htr = ftc->GetHypoTest();
         htr->Print();
         cout << "PLLTS " << htr->Significance() << endl;
         tmcs->SetTestStatistic(mlets);
         htr = ftc->GetHypoTest();
         htr->Print();
         cout << "MLETS " << htr->Significance() << endl;
         tmcs->SetTestStatistic(nevts);
         htr = ftc->GetHypoTest();
         htr->Print();
         cout << "NEVTS " << htr->Significance() << endl;
         tmcs->SetTestStatistic(roplts);
         htr = ftc->GetHypoTest();
         htr->Print();
         cout << "ROPLTS " << htr->Significance() << endl;

         regValue(htr->Significance(), "thtc_significance_frequentist");

         delete ftc;
         delete htr;
         delete w; // interesting why it doesn't work
         delete data;
      }
      return kTRUE ;
   }
} ;


class TestHypoTestCalculator3 : public RooUnitTest {
public:
   TestHypoTestCalculator3(TFile* refFile, Bool_t writeRef, Int_t verbose) : RooUnitTest("HypoTestCalculator Asymptotic - On / Off Problem", refFile, writeRef, verbose) {};

//   Double_t vtol() { return 0.2; } // tolerance may be too big

   Bool_t testCode() {

      const Int_t xValue = 150;
      const Int_t yValue = 100;
      const Double_t tauValue = 1;
      
      if(_write == kTRUE) {

         // register analytical Z_Bi value
         Double_t Z_Bi = NumberCountingUtils::BinomialWithTauObsZ(xValue, yValue, tauValue);
         regValue(Z_Bi, "thtc_significance_asymptotic");

      } else {
      
         // Make model for prototype on/off problem
         // Pois(x | s+b) * Pois(y | tau b )
         RooWorkspace* w = new RooWorkspace("w", kTRUE);
         w->factory("Poisson::on_pdf(x[150,0,500],sum::splusb(sig[0,0,100],bkg[100,0,300]))");
         w->factory("Poisson::off_pdf(y[100,0,500],prod::taub(tau[1],bkg))");
         w->factory("PROD::prod_pdf(on_pdf, off_pdf)");
        
         w->var("x")->setVal(xValue);
         w->var("y")->setVal(yValue);
         w->var("y")->setConstant();
         w->var("tau")->setVal(tauValue);
         w->var("tau")->setConstant();
         // construct the Bayesian-averaged model (eg. a projection pdf)
         // p'(x|s) = \int db p(x|s+b) * [ p(y|b) * prior(b) ]
         //w->factory("PROJ::averagedModel(PROD::foo(on_pdf|bkg,off_pdf,prior),bkg)") ;

         // define sets of variables obs={x} and poi={sig}
         // x is the only observable in the main measurement and y is treated as a separate measurement,
         // which is used to produce the prior that will be used in the calculation to randomize the nuisance parameters
         w->defineSet("obs", "x");
         w->defineSet("poi", "sig");
         w->defineSet("globObs", "y");

         // Add observable value to a data set
         RooDataSet *data = new RooDataSet("data", "data", *w->set("obs"));
         data->add(*w->set("obs"));

         // Build S+B and B models
         ModelConfig *sbModel = new ModelConfig("SB_ModelConfig", w);
         sbModel->SetPdf(*w->pdf("prod_pdf"));
         sbModel->SetObservables(*w->set("obs"));
       //  sbModel->SetGlobalObservables(*w->set("globObs"));      
         sbModel->SetParametersOfInterest(*w->set("poi"));
         w->var("sig")->setVal(xValue - yValue / tauValue); // important !
         sbModel->SetSnapshot(*w->set("poi"));

         ModelConfig *bModel = new ModelConfig("B_ModelConfig", w);
         bModel->SetPdf(*w->pdf("prod_pdf"));
         bModel->SetObservables(*w->set("obs"));
       //  bModel->SetGlobalObservables(*w->set("globObs"));
         bModel->SetParametersOfInterest(*w->set("poi"));
         w->var("sig")->setVal(0.0); // important !
         bModel->SetSnapshot(*w->set("poi"));


         // alternate priors
         w->factory("Gaussian::gauss_prior(bkg, y, expr::sqrty('sqrt(y)', y))");
         w->factory("Lognormal::lognorm_prior(bkg, y, expr::kappa('1+1./sqrt(y)',y))");

         AsymptoticCalculator *atc = new AsymptoticCalculator(*data, *sbModel, *bModel);
         
         HypoTestResult *htr = atc->GetHypoTest();
         htr->Print();
        
         regValue(htr->Significance(), "thtc_significance_asymptotic");

         delete atc;
         delete htr;
         delete w; // interesting why it doesn't work
         delete data;
      }
      return kTRUE ;
   }
} ;



///////////////////////////////////////////////////////////////////////////////
//
// PROFILE LIKELIHOOD INTERVAL - GAUSSIAN DISTRIBUTION
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

class TestProfileLikelihoodCalculator1 : public RooUnitTest {
public:
   TestProfileLikelihoodCalculator1(TFile* refFile, Bool_t writeRef, Int_t verbose) : RooUnitTest("ProfileLikelihoodCalculator Interval - Gaussian Model", refFile, writeRef, verbose) {} ;

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
         regValue(lowerLimit, "plc1_lower_limit_mean");
         regValue(upperLimit, "plc1_upper_limit_mean");

      } else {

         // Calculate likelihood interval using the ProfileLikelihoodCalculator
         RooArgSet *params = new RooArgSet();
         params->add(*w->var("mean"));
         ProfileLikelihoodCalculator *plc = new ProfileLikelihoodCalculator(*data, *w->pdf("gauss"), *params);
         plc->SetTestSize(testSize);

         LikelihoodInterval *interval = plc->GetInterval();

         // Register analytically computed limits in the reference file
         regValue(interval->LowerLimit(*w->var("mean")), "plc1_lower_limit_mean");
         regValue(interval->UpperLimit(*w->var("mean")), "plc1_upper_limit_mean");

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
// PROFILE LIKELIHOOD INTERVAL - POISSON DISTRIBUTION
//
// Test the 68% likelihood interval computed by the profile likelihood calculator
// on a Poisson distribution, from only one observed value. Reference values are
// computed via analytic methods: solve equation 2*[ln(LL(xMax)) - ln(LL(x))] = 1.
//
// 03/2012 - Ioan Gabriel Bucur
//
///////////////////////////////////////////////////////////////////////////////


class TestProfileLikelihoodCalculator2 : public RooUnitTest {
public:
   TestProfileLikelihoodCalculator2(TFile* refFile, Bool_t writeRef, Int_t verbose, Int_t obsValue = 0) : RooUnitTest(TString::Format("ProfileLikelihoodCalculator Interval - Poisson Simple Model - Observed value: %d", obsValue), refFile, writeRef, verbose), fObsValue(obsValue) {
      if (obsValue < 0) {
         fObsValue = 0;
         Warning("TestProfileLikelihoodCalculator2", "Negative observed value %d has been passed for Poisson distribution.\n   Using default observed value (0) instead.", obsValue);
      }
   };

   Bool_t testCode() {
      TString lowerLimitString = TString::Format("plc2_lower_limit_mean_%d", fObsValue);
      TString upperLimitString = TString::Format("plc2_upper_limit_mean_%d", fObsValue);

      // Put the significance level so that we obtain a 68% confidence interval
      const Double_t testSize = ROOT::Math::normal_cdf_c(1) * 2; // significance level

      // Create Poisson model and dataset
      RooWorkspace* w = new RooWorkspace("w", kTRUE);
      w->factory("Poisson::poiss(x[0,1000], mean[0,1000])");
      // NOTE: kTRUE mean of Poisson distribution does not really matter in this case
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

static pair<ModelConfig *, ModelConfig *> createPoissonProductModels(RooWorkspace *w)
{

   // Build models: background and s+b
   w->factory("Poisson::sb_poiss1(x[0,40], sum::splusb1(sig1[0,20], bkg1[0,20]))");
   w->factory("Uniform::prior(sig1)");
   w->factory("prod::sig2(2,sig1)");
   w->factory("Poisson::sb_poiss2(y[0,40], sum::splusb2(sig2, bkg2[0,20]))");
   w->factory("Poisson::b_poiss1(x, bkg1)");
   w->factory("Poisson::b_poiss2(y, bkg2)");
   w->factory("Poisson::constr1(gbkg1[2], bkg1)");
   w->factory("Poisson::constr2(gbkg2[3], bkg2)");
   w->factory("PROD::sb_pdf(sb_poiss1, constr1, sb_poiss2, constr2)");
   w->factory("PROD::b_pdf(b_poiss1, constr1, b_poiss2, constr2)");

   // build argument sets
   RooArgSet *obsSet =  new RooArgSet(*w->var("x"), *w->var("y"));
   RooArgSet *POISet = new RooArgSet(*w->var("sig1"));
   RooArgSet *NPSet = new RooArgSet(*w->var("bkg1"), *w->var("bkg2"));
   RooArgSet *globalObsSet = new RooArgSet(*w->var("gbkg1"), *w->var("gbkg2"));

   // build data set and import it into the workspace sets
   w->var("x")->setVal(7);
   w->var("y")->setVal(15);
   RooDataSet *data = new RooDataSet("data", "data", *obsSet);
   data->add(*obsSet);
   w->import(*data);

   // create model configuration
   ModelConfig *sbModel = new ModelConfig("SB_ModelConfig", w);
   sbModel->SetObservables(*obsSet);
   sbModel->SetParametersOfInterest(*POISet);
   sbModel->SetNuisanceParameters(*NPSet);
   sbModel->SetPdf("sb_pdf");
   sbModel->SetPriorPdf("prior");
   sbModel->SetSnapshot(*POISet);
   sbModel->SetGlobalObservables(*globalObsSet);

   // Background model -> sее createComplexModel for variable names
   ModelConfig *bModel = new ModelConfig(*sbModel);
   bModel->SetName("B_ModelConfig");
   bModel->SetPdf("b_pdf");
   w->var("sig1")->setVal(0);
   bModel->SetSnapshot(*POISet);
   
   return make_pair(sbModel, bModel);
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
   TestBayesianCalculator3(TFile* refFile, Bool_t writeRef, Int_t verbose) : RooUnitTest("BayesianCalculator Central Interval - Poisson Complex Product Model", refFile, writeRef, verbose) {};

   Bool_t testCode() {

      const Double_t testSize = ROOT::Math::normal_cdf_c(1) * 2; // test size
      // for the BayesianCalculator we need to change the integration method - default method is too slow
      RooAbsReal::defaultIntegratorConfig()->method1D().setLabel("RooAdaptiveGaussKronrodIntegrator1D");

      // Create workspace model
      RooWorkspace *w = new RooWorkspace("w", kTRUE);
      pair<ModelConfig *, ModelConfig *> models = createPoissonProductModels(w);

      // Create BayesianCalculator and calculate interval
      BayesianCalculator *bc = new BayesianCalculator(*w->data("data"), *models.first);
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
   TestMCMCCalculator(TFile* refFile, Bool_t writeRef, Int_t verbose) : RooUnitTest("MCMCCalculator Interval - Poisson Complex Model", refFile, writeRef, verbose) {};

   Bool_t testCode() {

      const Double_t testSize = ROOT::Math::normal_cdf_c(1) * 2;

      // Create workspace and model
      RooWorkspace *w = new RooWorkspace("w", kTRUE);
      pair<ModelConfig *, ModelConfig *> models = createPoissonProductModels(w);

      // build confidence interval (MCMCInterval) with MCMC calculator
      SequentialProposal *sp = new SequentialProposal(0.1);
      MCMCCalculator *mcmcc = new MCMCCalculator(*w->data("data"), *models.first);
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

class TestProfileLikelihoodCalculator3 : public RooUnitTest {
public:
   TestProfileLikelihoodCalculator3(TFile* refFile, Bool_t writeRef, Int_t verbose) : RooUnitTest("ProfileLikelihoodCalculator Interval - Poisson Complex Model", refFile, writeRef, verbose) {};

   Bool_t testCode() {

      const Double_t testSize = ROOT::Math::normal_cdf_c(1) * 2;

      // Create workspace and model
      RooWorkspace *w = new RooWorkspace("w", kTRUE);
      pair<ModelConfig *, ModelConfig *> models = createPoissonProductModels(w);

      // build likelihood interval with ProfileLikelihoodCalculator
      ProfileLikelihoodCalculator *plc = new ProfileLikelihoodCalculator(*w->data("data"), *models.first, testSize);
      LikelihoodInterval *interval = plc->GetInterval();
      regValue(interval->LowerLimit(*w->var("sig1")), "plc3_lower_limit_sig1");
      regValue(interval->UpperLimit(*w->var("sig1")), "plc3_upper_limit_sig1");

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
#include "RooStats/ProfileLikelihoodTestStat.h"
#include "RooStats/AsymptoticCalculator.h"

#include "RooStats/HypoTestInverterPlot.h"
#include "TCanvas.h"
#include "RooStats/SamplingDistPlot.h"


class TestHypoTestInverter1 : public RooUnitTest {
private:
   HypoTestInverter::ECalculatorType fCalculatorType;
   ETestStatType fTestStatType;

public:
   TestHypoTestInverter1(TFile* refFile, Bool_t writeRef, Int_t verbose, HypoTestInverter::ECalculatorType calculatorType, 
                         ETestStatType testStatType) : 
      RooUnitTest(TString::Format("HypoTestInverter Interval - Poisson Complex Model - %s %s", 
                                   kECalculatorTypeString[calculatorType], kETestStatTypeString[testStatType]), refFile, writeRef, verbose) {
      fCalculatorType = calculatorType;  
      fTestStatType = testStatType;
   };

   Bool_t testCode() {

      const Double_t testSize = ROOT::Math::normal_cdf_c(1) * 2;

      // Create workspace and model
      RooWorkspace *w = new RooWorkspace("w", kTRUE);
      pair<ModelConfig *, ModelConfig *> models = createPoissonProductModels(w);

      // configure HypoTestInverter
      HypoTestInverter *hti = new HypoTestInverter(*w->data("data"), *models.first, *models.second, NULL, fCalculatorType, testSize);
      hti->SetTestStatistic(*buildTestStatistic(fTestStatType, *models.first, *models.second));
      hti->SetFixedScan(10, 2, 8);
 
      //TODO: check how to eliminate this code, calculator should autoconfigure itself     
      if(fCalculatorType == HypoTestInverter::kHybrid) {
         // force prior nuisance pdf
         HybridCalculator *hc = (HybridCalculator *)hti->GetHypoTestCalculator();
         w->factory("PROD::priorbkg(constr1, constr2)");
         hc->ForcePriorNuisanceNull(*w->pdf("priorbkg"));
         hc->ForcePriorNuisanceAlt(*w->pdf("priorbkg"));
      }

      //TODO: check how this code can be eliminated, maybe 0 should be default print level for AsymptoticCalculator
      if(fCalculatorType == HypoTestInverter::kAsymptotic && _verb == 0) {
         AsymptoticCalculator::SetPrintLevel(0); // print only minimal output
      }

      // needed because we have no extended pdf and the ToyMC Sampler evaluation returns an error
      ToyMCSampler *tmcs = (ToyMCSampler *)hti->GetHypoTestCalculator()->GetTestStatSampler();
      tmcs->SetMaxToys(100);
      tmcs->SetNEventsPerToy(1);
      HypoTestInverterResult *interval = hti->GetInterval();
      regValue(interval->LowerLimit(), TString::Format("hti1_lower_limit_sig1_calc_%s_%s", 
                                                       kECalculatorTypeString[fCalculatorType],
                                                       kETestStatTypeString[fTestStatType] ));
      regValue(interval->UpperLimit(), TString::Format("hti1_upper_limit_sig1_calc_%s_%s",
                                                       kECalculatorTypeString[fCalculatorType],
                                                       kETestStatTypeString[fTestStatType] ));

      if(_verb >= 1) {
         cout << "[" << interval->LowerLimit() << "," << interval->UpperLimit() << "]" << endl;
         HypoTestInverterPlot *plot = new HypoTestInverterPlot("hti1 Plot", "Feldman-Cousins Interval", interval);
         TCanvas *c1 = new TCanvas("HypoTestInverter1 Scan");
         c1->SetLogy(false);
         plot->Draw("OBS");
         c1->SaveAs(TString::Format("hti1 Scan - %s %s.pdf",
                                    kECalculatorTypeString[fCalculatorType],
                                    kETestStatTypeString[fTestStatType] ));

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
                  if(pl == NULL) return kTRUE;
                  pl->SetLogYaxis(kTRUE);
                  pl->Draw();
               }
               c2->SaveAs(TString::Format("hti1 TestStatDistributions - %s %s.pdf",
                                          kECalculatorTypeString[fCalculatorType],
                                          kETestStatTypeString[fTestStatType] ));
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


static pair<ModelConfig *, ModelConfig *> createPoissonSBEModels(RooWorkspace *w) {
   
   // build models 
   w->factory("Gaussian::constrb(b0[-5,5], b1[-5,5], 1)");
   w->factory("Gaussian::constre(e0[-5,5], e1[-5,5], 1)");
   w->factory("cexpr::bkg('5 * pow(1.3, b1)', b1)"); // background model
   w->factory("cexpr::eff('0.5 * pow(1.2, e1)', e1)"); // efficiency model
   w->factory("cexpr::splusb('eff * sig + bkg', eff, bkg, sig[0,20])");
   w->factory("Poisson::sb_poiss(x[0,40], splusb)");
   w->factory("Poisson::b_poiss(x, bkg)");
   w->factory("PROD::sb_pdf(sb_poiss, constrb, constre)");
   w->factory("PROD::b_pdf(b_poiss, constrb)");

   w->var("b0")->setConstant(kTRUE);
   w->var("e0")->setConstant(kTRUE);

   // build argument sets
   RooArgSet *obsSet =  new RooArgSet(*w->var("x"));
   RooArgSet *POISet = new RooArgSet(*w->var("sig"));
   RooArgSet *NPSet = new RooArgSet(*w->var("b1"), *w->var("e1"));
   RooArgSet *globalObsSet = new RooArgSet(*w->var("b0"), *w->var("e0"));

   // build data set and import it into the workspace sets
   w->var("x")->setVal(8);
   RooDataSet *data = new RooDataSet("data", "data", *obsSet); 
   data->add(*obsSet);
   w->import(*data);

   // create model configuration
   ModelConfig *sbModel = new ModelConfig("SB_ModelConfig", w);
   sbModel->SetObservables(*obsSet);
   sbModel->SetParametersOfInterest(*POISet);
   sbModel->SetNuisanceParameters(*NPSet);
   sbModel->SetPdf("sb_pdf");
   //sbModel->SetPriorPdf("prior");
   sbModel->SetSnapshot(*POISet);
   sbModel->SetGlobalObservables(*globalObsSet);
 
   ModelConfig *bModel = new ModelConfig(*sbModel);
   bModel->SetName("B_ModelConfig");
   bModel->SetPdf("b_pdf");
   w->var("sig")->setVal(1);
   bModel->SetSnapshot(*POISet);
      
   return make_pair(sbModel, bModel);
}


class TestHypoTestInverter2 : public RooUnitTest {
private:
   HypoTestInverter::ECalculatorType fCalculatorType;

public:
   TestHypoTestInverter2(TFile* refFile, Bool_t writeRef, Int_t verbose, HypoTestInverter::ECalculatorType calculatorType, ETestStatType testStatType) : RooUnitTest(TString::Format("HypoTestInverter Upper Limit - Poisson Model with Signal, Background and Efficiency - Calculator Type %d", calculatorType), refFile, writeRef, verbose) {
      fCalculatorType = calculatorType;  
   };

   Bool_t testCode() {

      const Double_t testSize = ROOT::Math::normal_cdf_c(1) * 2;

      // Create workspace and model
      RooWorkspace *w = new RooWorkspace("w", kTRUE);
      pair<ModelConfig *, ModelConfig *> models = createPoissonSBEModels(w);


      // calculate upper limit with HypoTestInverter
      HypoTestInverter *hti = new HypoTestInverter(*w->data("data"), *models.first, *models.second, (RooRealVar *)models.first->GetParametersOfInterest()->first(), fCalculatorType, testSize);
      hti->SetFixedScan(10, 0, 20);
      hti->UseCLs(kTRUE);
      
      if(fCalculatorType == HypoTestInverter::kAsymptotic && _verb == 0) {
         AsymptoticCalculator::SetPrintLevel(0); // print only minimal output
      }

      if(fCalculatorType == HypoTestInverter::kHybrid) {
         // force prior nuisance pdf
         HybridCalculator *hc = (HybridCalculator *)hti->GetHypoTestCalculator();
         w->factory("PROD::priorbkg(constrb, constre)");
         hc->ForcePriorNuisanceNull(*w->pdf("priorbkg"));
         hc->ForcePriorNuisanceAlt(*w->pdf("priorbkg"));
      }

      // Set up the test statistic
      ProfileLikelihoodTestStat *profll = new ProfileLikelihoodTestStat(*models.first->GetPdf());
      profll->SetOneSided(kTRUE);

      // needed because we have no extended pdf and the ToyMC Sampler evaluation returns an error
      ToyMCSampler *tmcs = (ToyMCSampler *)hti->GetHypoTestCalculator()->GetTestStatSampler();
      tmcs->SetMaxToys(300);
      tmcs->SetNEventsPerToy(1);
      tmcs->SetTestStatistic(profll);
//      tmcs->SetUseMultiGen(kTRUE); // make ToyMCSampler faster   
  
      // calculate interval and extract observed upper limit and expected upper limit (+- sigma)
      HypoTestInverterResult *interval = hti->GetInterval();
      regValue(interval->UpperLimit(), TString::Format("hti2_upper_limit_sig1_calc_%d", fCalculatorType));
      regValue(interval->GetExpectedUpperLimit(0), TString::Format("hti2_exp_upper_limit_sig_calc_%d", fCalculatorType));
      regValue(interval->GetExpectedUpperLimit(-1), TString::Format("hti2_exp_upper_limit-sigma_sig_calc_%d", fCalculatorType));
      regValue(interval->GetExpectedUpperLimit(1), TString::Format("hti2_exp_upper_limit+sigma_sig_calc_%d", fCalculatorType));


      if(_verb >= 1) {
         cout << "[" << interval->LowerLimit() << "," << interval->UpperLimit() << "]" << endl;
         HypoTestInverterPlot *plot = new HypoTestInverterPlot("HTI_Result_Plot", "HTI Upper Limit Scan", interval);
         TCanvas *c1 = new TCanvas("HypoTestInverter Scan");
         c1->SetLogy(false);
         plot->Draw("2CL CLb");
         c1->SaveAs(TString::Format("hti2 Upper Limit Scan - Calc %d.pdf", fCalculatorType));

 
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
                  if(pl == NULL) return kTRUE;
                  pl->SetLogYaxis(kTRUE);
                  pl->Draw();
               }
               c2->SaveAs(TString::Format("hti2 TestStat Distributions - Calc %d.pdf", fCalculatorType));
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

#include "RooStats/RatioOfProfiledLikelihoodsTestStat.h"
#include "RooStats/MaxLikelihoodEstimateTestStat.h"
#include "RooStats/NumEventsTestStat.h"

static TestStatistic *buildTestStatistic(const ETestStatType testStatType, const ModelConfig &sbModel, const ModelConfig &bModel) {

   TestStatistic *testStat = NULL;
 
   if(testStatType == kSimpleLR) {
      SimpleLikelihoodRatioTestStat *slrts = new SimpleLikelihoodRatioTestStat(*sbModel.GetPdf(), *bModel.GetPdf());
      slrts->SetNullParameters(*sbModel.GetSnapshot());
      slrts->SetAltParameters(*bModel.GetSnapshot());
      slrts->SetReuseNLL(kTRUE);
      testStat = slrts;
   } else if(testStatType == kRatioLR)  {
      RatioOfProfiledLikelihoodsTestStat *roplts = 
         new RatioOfProfiledLikelihoodsTestStat(*sbModel.GetPdf(), *bModel.GetPdf(), bModel.GetSnapshot());
      roplts->SetReuseNLL(kTRUE);
      testStat = roplts;
   } else if(testStatType == kMLE) {
      MaxLikelihoodEstimateTestStat *mlets = 
         new MaxLikelihoodEstimateTestStat(*sbModel.GetPdf(), *((RooRealVar *)sbModel.GetParametersOfInterest()->first()));
      testStat = mlets;
   } else if(testStatType == kNObs) {
      NumEventsTestStat *nevtts = new NumEventsTestStat(*sbModel.GetPdf());
      testStat = nevtts;
   } else { // kProfileLR, kProfileLROneSided and kProfileLRSigned
      ProfileLikelihoodTestStat *plts = new ProfileLikelihoodTestStat(*sbModel.GetPdf());
      if(testStatType == kProfileLROneSided) plts->SetOneSided(kTRUE);
      if(testStatType == kProfileLRSigned) plts->SetSigned(kTRUE);      
      plts->SetReuseNLL(kTRUE);
      testStat = plts;
   }

   assert(testStat != NULL); // sanity check - should never happen

   return testStat; // fgNToys seems like a good choice for now
}




