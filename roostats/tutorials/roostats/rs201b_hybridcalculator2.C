#include "RooRandom.h"
#include "RooRealVar.h"
#include "RooGaussian.h"
#include "RooPolynomial.h"
#include "RooArgSet.h"
#include "RooAddPdf.h"
#include "RooDataSet.h"
#include "RooExtendPdf.h"
#include "RooConstVar.h"

#ifndef __CINT__  // problem including this file with CINT
#include "RooGlobalFunc.h"
#endif

#include "RooStats/HybridCalculator2.h"
#include "RooStats/ToyMCSampler2.h"
#include "RooStats/ProfileLikelihoodTestStat.h"
#include "RooStats/SimpleLikelihoodRatioTestStat.h"
#include "RooStats/RatioOfProfiledLikelihoodsTestStat.h"
#include "RooStats/HypoTestPlot.h"
#include "RooStats/HypoTestResult.h"


void rs201b_hybridcalculator2(int ntoys = 1000)
{
  //***********************************************************************//
  // This macro show an example on how to use RooStats/HybridCalculator    //
  //***********************************************************************//
  //
  // With this example, you should get: CL_sb = 0.130 and CL_b = 0.946
  // (if data had -2lnQ = -3.0742). You can compare to the expected plot:



  using namespace RooFit;
  using namespace RooStats;

  /// set RooFit random seed
  RooRandom::randomGenerator()->SetSeed(3007);

  /// build the models for background and signal+background
  RooRealVar x("x","",-3,3);
  RooArgSet observables(x); // variables to be generated

  // gaussian signal
  RooRealVar sig_mean("sig_mean","",0);
  RooRealVar sig_sigma("sig_sigma","",0.8);
  RooGaussian sig_pdf("sig_pdf","",x,sig_mean,sig_sigma);
//  RooGaussian sig_pdf("sig_pdf","",x, RooConst(0.0),RooConst(0.8));
  RooRealVar sig_yield("sig_yield","",20,0,300);

  // flat background (extended PDF)
  RooRealVar bkg_slope("bkg_slope","",0);
  RooPolynomial bkg_pdf("bkg_pdf","",x,bkg_slope);
//  RooPolynomial bkg_pdf("bkg_pdf","", x, RooConst(0));
  RooRealVar bkg_yield("bkg_yield","",40,0,300);
  //bkg_yield.setBins(ntoys); // TODO How to set this automatically for sets of nuisance parameters?
  RooExtendPdf bkg_ext_pdf("bkg_ext_pdf","",bkg_pdf,bkg_yield);

  // total sig+bkg (extended PDF)
  RooAddPdf tot_pdf("tot_pdf","",RooArgList(sig_pdf,bkg_pdf),RooArgList(sig_yield,bkg_yield));

  /// build the prior PDF on the parameters to be integrated
  // gaussian constraint on the background yield ( N_B = 40 +/- 10  ie. 25% )
  RooGaussian bkg_yield_prior("bkg_yield_prior","",bkg_yield,RooConst(bkg_yield.getVal()),RooConst(10.));

  RooArgSet nuisance_parameters(bkg_yield); // variables to be integrated (not used as determined automatically)
  RooArgSet parametersOfInterest(sig_yield);

  /// generate a data sample
  RooDataSet* data = tot_pdf.generate(observables,RooFit::Extended());

  /*
  RooPlot* frame = x.frame();
  data->plotOn(frame);
  tot_pdf.plotOn(frame);
  frame->Draw();
  */
  //***********************************************************************//



  // 2lnQ: -3.15469
  // -lnQ: 1.57734
  // Note about ModelConfig: The HybridCalculator2 uses its knowledge about the data
  // to determine observables and nuisance parameters automatically using
  // the GuessObsAndNuisance(...) function.
  RooWorkspace w;
  ModelConfig sb_model("S+B_model");
  sb_model.SetWorkspace(w);
  sb_model.SetPdf(tot_pdf);
  sb_model.SetParameters(parametersOfInterest);
  sb_model.SetSnapshot(parametersOfInterest);

  ModelConfig b_model("B_model");
  b_model.SetWorkspace(w);
  b_model.SetPdf(tot_pdf);
  b_model.SetParameters(parametersOfInterest);
  sig_yield.setVal(0.0);
  b_model.SetSnapshot(parametersOfInterest);
  //ropl.SetNullVars(parametersOfInterest);

//  bkg_yield.setConstant(kTRUE);
//  w.Print();

  // test statistic options
  SimpleLikelihoodRatioTestStat slrts(tot_pdf, tot_pdf);
  slrts.SetNullParameters(*b_model.GetSnapshot());
  slrts.SetAltParameters(*sb_model.GetSnapshot());

  RatioOfProfiledLikelihoodsTestStat ropl(tot_pdf, tot_pdf, sb_model.GetSnapshot());
  ropl.SetSubtractMLE(false);
  ProfileLikelihoodTestStat profll(tot_pdf);
  MaxLikelihoodEstimateTestStat mlets(tot_pdf, sig_yield);

  // Create toyMCSampler with chosen test statistic
  //  ToyMCSampler2 toymcsampler2(slrts, ntoys);
  //  ToyMCSampler2 toymcsampler2(profll, ntoys);
  ToyMCSampler2 toymcsampler2(ropl, ntoys);
  //  ToyMCSampler2 toymcsampler2(mlets, ntoys);

  //toymcsampler2.SetExpectedNuisancePar(kTRUE);
  //toymcsampler2.SetGenerateBinned(true);
  //toymcsampler2.SetNEventsPerToy(10000);

  HybridCalculator2 myH2(*data,sb_model, b_model, &toymcsampler2);
  //HybridCalculator2 myH2(*data,sb_model, b_model);
  myH2.ForcePriorNuisanceNull(bkg_yield_prior); // ad hoc hybrid
  myH2.ForcePriorNuisanceAlt(bkg_yield_prior);  // ad hoc hybrid
  HypoTestResult *res = myH2.GetHypoTest();
  res->Print();
  HypoTestPlot *plot = new HypoTestPlot(*res, 80); // number of bins is optional (default: 100)
  plot->Draw();

  TCanvas *c = new TCanvas("rs201b_hybridcalculator2","rs201b_hybridcalculator2");
  HypoTestPlot *plot = new HypoTestPlot(*res, 80); // number of bins is optional (default: 100)
  plot->Draw();
//  plot->DumpToFile("test.root", "RECREATE");
//  c->Print("rs201b_output.pdf");

  return;
}


