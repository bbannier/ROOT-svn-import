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
#include "RooStats/HybridResult.h"
#include "RooStats/HybridPlot.h"
#include "RooStats/ToyMCSampler2.h"
#include "RooStats/ProfileLikelihoodTestStat.h"
#include "RooStats/SimpleLikelihoodRatioTestStat.h"


void rs201b_hybridcalculator2(int ntoys = 3000)
{
  //***********************************************************************//
  // This macro show an example on how to use RooStats/HybridCalculator    //
  //***********************************************************************//
  //
  // With this example, you should get: CL_sb = 0.130 and CL_b = 0.946
  // (if data had -2lnQ = -3.0742). You can compare to the expected plot:
  // http://www-ekp.physik.uni-karlsruhe.de/~schott/roostats/hybridplot_example.png

  using namespace RooFit;
  using namespace RooStats;

  /// set RooFit random seed
  RooRandom::randomGenerator()->SetSeed(3007);

  /// build the models for background and signal+background
  RooRealVar x("x","",-3,3);
  RooArgSet observables(x); // variables to be generated

  // gaussian signal
//  RooRealVar sig_mean("sig_mean","",0);
//  RooRealVar sig_sigma("sig_sigma","",0.8);
//  RooGaussian sig_pdf("sig_pdf","",x,sig_mean,sig_sigma);
  RooGaussian sig_pdf("sig_pdf","",x, RooConst(0.0),RooConst(0.8));
  RooRealVar sig_yield("sig_yield","",20,0,300);

  // flat background (extended PDF)
//  RooRealVar bkg_slope("bkg_slope","",0);
//  RooPolynomial bkg_pdf("bkg_pdf","",x,bkg_slope);
  RooPolynomial bkg_pdf("bkg_pdf","", x, RooConst(0));
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

  //***********************************************************************//


  SimpleLikelihoodRatioTestStat slrts(bkg_ext_pdf, tot_pdf);
  ProfileLikelihoodTestStat profll(tot_pdf);

  // 2lnQ: -3.15469
  // -lnQ: 1.57734
  RooWorkspace w;
  ModelConfig sb_model("sbmodel");
  sb_model.SetWorkspace(w);
  sb_model.SetPdf(tot_pdf);
  sb_model.SetParameters(parametersOfInterest);
  //sb_model.SetObservables(observables); // determined automatically
  //sb_model.SetNuisanceParameters(nuisance_parameters); // determined automatically
  sb_model.SetPriorPdf(bkg_yield_prior);
  sb_model.SetSnapshot(parametersOfInterest);

  ModelConfig b_model("bmodel");
  b_model.SetWorkspace(w);
  b_model.SetPdf(tot_pdf);
  b_model.SetParameters(parametersOfInterest);
  //b_model.SetObservables(observables); // determined automatically
  //b_model.SetNuisanceParameters(nuisance_parameters); // determined automatically
  b_model.SetPriorPdf(bkg_yield_prior);
  sig_yield.setVal(0.0);
  b_model.SetSnapshot(parametersOfInterest);
  //sig_yield.setVal(20.0);

  bkg_yield.setConstant(kTRUE);
  //sig_yield.setConstant(kTRUE);


  ToyMCSampler2 toymcsampler2(slrts, ntoys);
  //ToyMCSampler2 toymcsampler2(profll, ntoys);
  //ToyMCSampler2 toymcsampler2(profllr, ntoys);
  //toymcsampler2.SetExpectedNuisancePar(kTRUE);

  HybridCalculator2 myH2("H2", toymcsampler2, sb_model, b_model, *data);
  HybridResult *res = myH2.GetHypoTest();
  HybridPlot* myHybridPlot = res->GetPlot("myHybridPlot","Results with HybridCalculator2",100);
  myHybridPlot->Draw();
  res->PrintMore("");

  return;
}


