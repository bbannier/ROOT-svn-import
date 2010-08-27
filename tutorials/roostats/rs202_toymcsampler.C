#include "RooRandom.h"

#include "RooStats/ToyMCSampler.h"
#include "RooStats/ProfileLikelihoodTestStat.h"
#include "RooStats/SamplingDistribution.h"
#include "RooStats/SamplingDistPlot.h"

using namespace RooFit;
using namespace RooStats;

void rs202_toymcsampler(int toysPerWorker=100)
{
  //***********************************************************************//
  // This macro shows an example on how to use RooStats/ToyMCSampler       //
  // Tutorial by Kyle Cranmer and Sven Kreiss                              //
  //***********************************************************************//
  //
  // Simple demonstration of ToyMCSampler including option for Proof.

  // set RooFit random seed for reproducible results
  RooRandom::randomGenerator()->SetSeed(3008);

  // workspace
  RooWorkspace w("example");

  // Gaussian signal over observable x
  w.factory("Gaussian::sig_pdf(x[-3,3],sig_mean[0],sig_sigma[1])");
  // flat background
  w.factory("Uniform::bkg_pdf(x)");
  // total model with signal and background yields as parameters
  w.factory("SUM::main_pdf(sig_yield[20,0,300]*sig_pdf,bkg_yield[50,0,300]*bkg_pdf)");

  // The model for the control sample that constrains the background.
  w.factory("Gaussian::control_pdf(control_meas[50],bkg_yield,10.)");


  // define sets for reference later
  w.defineSet("obs","x");
  w.defineSet("poi","sig_yield");

  // Test Statistic: profile likelihood where alternate is best fit value of signal yield
  ProfileLikelihoodTestStat profll(*w.pdf("main_pdf"));

  // ToyMCSampler
  ToyMCSampler toymcsampler(profll, toysPerWorker);
  toymcsampler.SetObservables(*w.set("obs"));
  toymcsampler.SetParametersForTestStat(*w.set("poi"));
  toymcsampler.SetPriorNuisance(w.pdf("control_pdf"));
  toymcsampler.SetPdf(*w.pdf("main_pdf"));

  // Proof
  ProofConfig pc(w, 10, "workers=2");    // proof-lite
  //ProofConfig pc(w, 8, "localhost");    // proof cluster at "localhost"
  toymcsampler.SetProofConfig(&pc);     // enable proof

  // run the ToyMCSampler
  RooArgSet poi(*w.set("poi"));
  SamplingDistribution *result = toymcsampler.GetSamplingDistribution(poi);

  // plot result
  SamplingDistPlot *plot = new SamplingDistPlot();
  plot->AddSamplingDistribution(result);
  plot->SetLineWidth(2, result);
  plot->SetLineColor(kRed, result);
  plot->Draw();
}


