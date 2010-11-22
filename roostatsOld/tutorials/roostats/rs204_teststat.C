#include "RooStats/ProfileLikelihoodTestStat.h"
#include "RooStats/SimpleLikelihoodRatioTestStat.h"
#include "RooStats/RatioOfProfiledLikelihoodsTestStat.h"
#include "RooStats/MaxLikelihoodEstimateTestStat.h"

using namespace RooFit;
using namespace RooStats;

int testTestStat(TestStatistic& t, RooAbsData& data, const RooArgSet& nullPoi, RooArgSet& p) {
   // tests data encapsulation of a TestStatistic and returns
   // how often it failed

   int fails = 0;
   double ref = t.Evaluate(data, nullPoi);
   double val = ref;
   double threshold = 0.01;

   // test 1
   p.setRealValue("x", RooRandom::uniform()*6.0-3.0);
   val = t.Evaluate(data, nullPoi);
   if(fabs(val) > fabs(ref*(1.0+threshold))  ||  fabs(val) < fabs(ref*(1.0-threshold))) { cout << val << " != " << ref << endl; fails++; }

   // test 2
   p.setRealValue("sig_yield", RooRandom::uniform()*50.0);
   //p.Print("v");
   val = t.Evaluate(data, nullPoi);
   //p.Print("v");
   if(fabs(val) > fabs(ref*(1.0+threshold))  ||  fabs(val) < fabs(ref*(1.0-threshold))) { cout << val << " != " << ref << endl; fails++; }

   // test 3
   p.setRealValue("bkg_yield", RooRandom::uniform()*50.0);
   val = t.Evaluate(data, nullPoi);
   if(fabs(val) > fabs(ref*(1.0+threshold))  ||  fabs(val) < fabs(ref*(1.0-threshold))) { cout << val << " != " << ref << endl; fails++; }

   return fails;
}

void rs204_teststat()
{
  //***********************************************************************//
  // Tests for TestStatistics                                              //
  // Tutorial by Kyle Cranmer and Sven Kreiss                              //
  //***********************************************************************//

  // set RooFit random seed for reproducible results
  //RooRandom::randomGenerator()->SetSeed(3008);
  RooRandom::randomGenerator()->SetSeed(5);

  // workspace
  RooWorkspace w("example");

  // Gaussian signal over observable x
  w.factory("Gaussian::sig_pdf(x[-3,3],sig_mean[0],sig_sigma[1])");
  // flat background
  w.factory("Uniform::bkg_pdf(x)");
  // total model with signal and background yields as parameters
  w.factory("SUM::main_pdf(sig_yield[20,0,300]*sig_pdf,bkg_yield[50,0,300]*bkg_pdf)");
  // define sets for reference later
  w.defineSet("obs","x");
  w.defineSet("poi","sig_yield");
  w.defineSet("nuis","bkg_yield");
  w.defineSet("pando", "x,sig_yield,bkg_yield"); // parameters and observables

  // D E F I N E  N U L L  &  A L T E R N A T I V E   H Y P O T H E S E S
  ModelConfig b_model("B_model", &w);
  b_model.SetPdf(*w.pdf("main_pdf"));
  w.var("sig_yield")->setVal(0.0);
  b_model.SetSnapshot(*w.set("poi"));

  ModelConfig sb_model("S+B_model", &w);
  sb_model.SetPdf(*w.pdf("main_pdf"));
  w.var("sig_yield")->setVal(20.0);
  sb_model.SetSnapshot(*w.set("poi"));



  // a simple likelihood ratio with background fixed to nominal value
  SimpleLikelihoodRatioTestStat slrts(*b_model.GetPdf(), *sb_model.GetPdf());
  slrts.SetNullParameters(*b_model.GetSnapshot());
  slrts.SetAltParameters(*sb_model.GetSnapshot());

  // ratio of alt and null likelihoods with background yiled profiled
  RatioOfProfiledLikelihoodsTestStat ropl(*b_model.GetPdf(), *sb_model.GetPdf(), sb_model.GetSnapshot());

  // profile likelihood where alternate is best fit value of signal yield
  ProfileLikelihoodTestStat profll(*sb_model.GetPdf());

  // just use the maximum likelihood estimate of signal yield
  MaxLikelihoodEstimateTestStat mlets(*sb_model.GetPdf(), *w.var("sig_yield"));



  RooAbsData *data = sb_model.GetPdf()->generate(*w.set("obs"));
  cout << "SLRTS:  " << testTestStat(slrts,  *data, *b_model.GetSnapshot(), *w.set("pando")) << endl;
  cout << "ROPL (default):  " << testTestStat(ropl,   *data, *b_model.GetSnapshot(), *w.set("pando")) << endl;
  ropl.SetSubtractMLE(false);
  cout << "ROPL (no MLE):   " << testTestStat(ropl,   *data, *b_model.GetSnapshot(), *w.set("pando")) << endl;
  cout << "PROFLL: " << testTestStat(profll, *data, *b_model.GetSnapshot(), *w.set("pando")) << endl;
  cout << "MLETS:  " << testTestStat(mlets,  *data, *b_model.GetSnapshot(), *w.set("pando")) << endl;
}


