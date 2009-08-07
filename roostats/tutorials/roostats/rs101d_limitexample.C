/////////////////////////////////////////////////////////////////////////
//
// 'Limit Example' RooStats tutorial macro #101
// author: Kyle Cranmer
// date June. 2009
//
// This tutorial shows an example of creating a simple
// model for a number counting experiment with uncertainty
// on both the background rate and signal efficeincy. We then 
// use a Confidence Interval Calculator to set a limit on the signal.
//
//
/////////////////////////////////////////////////////////////////////////

#ifndef __CINT__
#include "RooGlobalFunc.h"
#endif

#include "RooStats/ProfileLikelihoodCalculator.h"
#include "RooStats/MCMCCalculator.h"
#include "RooStats/MCMCInterval.h"
#include "RooStats/MCMCIntervalPlot.h"
#include "RooStats/ProposalFunction.h"
#include "RooStats/FeldmanCousins.h"
#include "RooStats/NumberCountingPdfFactory.h"
#include "RooStats/ConfInterval.h"
#include "RooStats/PointSetInterval.h"
#include "RooStats/LikelihoodInterval.h"
#include "RooStats/LikelihoodIntervalPlot.h"
#include "RooStats/PdfProposal.h"
#include "RooStats/ProposalHelper.h"
#include "RooProfileLL.h"
#include "RooFitResult.h"
#include "RooAbsPdf.h"
#include "RooStats/HypoTestResult.h"
#include "RooRealVar.h"
#include "RooPlot.h"
#include "RooDataSet.h"
#include "RooTreeDataStore.h"
#include "TTree.h"
#include "TCanvas.h"
#include "TLine.h"
#include "TStopwatch.h"
#include "TFile.h"
#include "TMatrixDSym.h"
#include "TMatrixTSym.h"

#include "RooStats/RooStatsUtils.h"
#include <vector>

// use this order for safety on library loading
using namespace RooFit ;
using namespace RooStats ;


void rs101d_limitexample()
{
  /////////////////////////////////////////
  // An example of setting a limit in a number counting experiment with uncertainty on background and signal
  /////////////////////////////////////////

  // to time the macro
  TStopwatch t;
  t.Start();

  /////////////////////////////////////////
  // The Model building stage
  /////////////////////////////////////////
  RooWorkspace* wspace = new RooWorkspace();
  wspace->factory("Poisson::countingModel(obs[150,0,300], sum(s[50,0,120]*ratioSigEff[1.,0,2.],b[100,0,300]*ratioBkgEff[1.,0.,2.]))"); // counting model
  wspace->factory("Gaussian::sigConstraint(ratioSigEff,1,0.05)"); // 5% signal efficiency uncertainty
  wspace->factory("Gaussian::bkgConstraint(ratioBkgEff,1,0.1)"); // 10% background efficiency uncertainty
  wspace->factory("PROD::modelWithConstraints(countingModel,sigConstraint,bkgConstraint)"); // product of terms
  wspace->Print();

  RooAbsPdf* modelWithConstraints = wspace->pdf("modelWithConstraints"); // get the model
  RooRealVar* obs = wspace->var("obs"); // get the observable
  RooRealVar* s = wspace->var("s"); // get the signal we care about
  RooRealVar* b = wspace->var("b"); // get the background and set it to a constant.  Uncertainty included in ratioBkgEff
  b->setConstant();
  RooRealVar* ratioSigEff = wspace->var("ratioSigEff"); // get uncertaint parameter to constrain
  RooRealVar* ratioBkgEff = wspace->var("ratioBkgEff"); // get uncertaint parameter to constrain
  RooArgSet constrainedParams(*ratioSigEff, *ratioBkgEff); // need to constrain these in the fit (should change default behavior)

  // Create an example dataset with 160 observed events
  obs->setVal(160.);
  RooDataSet* data = new RooDataSet("exampleData", "exampleData", RooArgSet(*obs));
  data->add(*obs);

  RooArgSet all(*s, *ratioBkgEff, *ratioSigEff);

  // not necessary
  RooFitResult* fitResult = modelWithConstraints->fitTo(*data, RooFit::Constrain(RooArgSet(*ratioSigEff, *ratioBkgEff)), Save());

  // Now let's make some confidence intervals for s, our parameter of interest
  RooArgSet paramOfInterest(*s);

  // First, let's use a Calculator based on the Profile Likelihood Ratio
  ProfileLikelihoodCalculator plc;
  plc.SetPdf(*modelWithConstraints);
  plc.SetData(*data); 
  plc.SetParameters( paramOfInterest );
  plc.SetTestSize(.1);
  ConfInterval* lrint = plc.GetInterval();  // that was easy.

  // Second, use a Calculator based on the Feldman Cousins technique
  FeldmanCousins fc;
  fc.SetPdf(*modelWithConstraints);
  fc.SetData(*data); 
  fc.SetParameters( paramOfInterest );
  fc.UseAdaptiveSampling(true);
  fc.FluctuateNumDataEntries(false); // number counting analysis: dataset always has 1 entry with N events observed
  fc.SetNBins(5); // number of points to test per parameter
  fc.SetTestSize(.1);
  //  fc.SaveBeltToFile(true); // optional
  ConfInterval* fcint = NULL;
  //fcint = fc.GetInterval();  // that was easy.

  // Third, use a Calculator based on Markov Chain monte carlo
  //MCMCCalculator mc(*data, *modelWithConstraints, paramOfInterest);

  // Get a proposal function
  ProposalHelper ph;
  ph.SetVariables(fitResult->floatParsFinal());
  ph.SetCovMatrix(fitResult->covarianceMatrix());
  ph.SetUpdateProposalParameters(kTRUE);
  //ph.SetUniformFraction(0.05);
  ph.SetCacheSize(100);
  ProposalFunction* pf = ph.GetProposalFunction();

  MCMCCalculator mc;
  mc.SetData(*data);
  mc.SetPdf(*modelWithConstraints);
  mc.SetParameters(paramOfInterest);
  mc.SetProposalFunction(*pf);
  mc.SetNumIters(100000); // steps in the chain
  mc.SetNumBurnInSteps(50);
  mc.SetNumBins(50);
  mc.SetTestSize(.1); // 90% CL
  mc.SetUseKeys(kTRUE);
  mc.SetUseSparseHist(kTRUE);
  ConfInterval* mcmcint = NULL;
  mcmcint = mc.GetInterval();

  printf("Acutal confidence level = %g\n", ((MCMCInterval*)mcmcint)->GetActualConfidenceLevel());
  printf("hist cutoff = %g\n", ((MCMCInterval*)mcmcint)->GetHistCutoff());

  // Let's make a plot
  TCanvas* dataCanvas = new TCanvas("dataCanvas");
  dataCanvas->Divide(2,1);

  dataCanvas->cd(1);
  LikelihoodIntervalPlot plotInt((LikelihoodInterval*)lrint);
  plotInt.SetTitle("Profile Likelihood Ratio and Posterior for S");
  plotInt.Draw();
  dataCanvas->Update();

  MCMCIntervalPlot mcPlot(*((MCMCInterval*)mcmcint));

  // draw posterior
  //mcPlot.Draw("same");
  mcPlot.SetLineColor(kOrange);
  mcPlot.SetLineWidth(2);
  mcPlot.DrawHistInterval("same");
  mcPlot.SetLineWidth(1);

  //mcPlot.DrawPosterior("same");
  mcPlot.DrawPosteriorHist("same");
  mcPlot.DrawPosteriorKeysPdf("same");
  mcPlot.DrawPosteriorKeysProduct("same");

  mcPlot.SetLineColor(kMagenta);
  mcPlot.SetLineWidth(2);
  mcPlot.DrawKeysPdfInterval("same");
  dataCanvas->Update();
  mcPlot.SetLineWidth(1);

  // Get Lower and Upper limits from Profile Calculator
  cout << "Profile lower limit on s = " << ((LikelihoodInterval*) lrint)->LowerLimit(*s) << endl;
  cout << "Profile upper limit on s = " << ((LikelihoodInterval*) lrint)->UpperLimit(*s) << endl;

  // Get Lower and Upper limits from FeldmanCousins with profile construction
  if (fcint != NULL) {
     double fcul = ((PointSetInterval*) fcint)->UpperLimit(*s);
     double fcll = ((PointSetInterval*) fcint)->LowerLimit(*s);
     cout << "FC lower limit on s = " << fcll << endl;
     cout << "FC upper limit on s = " << fcul << endl;
     TLine* fcllLine = new TLine(fcll, 0, fcll, 1);
     TLine* fculLine = new TLine(fcul, 0, fcul, 1);
     fcllLine->SetLineColor(kRed);
     fculLine->SetLineColor(kRed);
     fcllLine->Draw("same");
     fculLine->Draw("same");
     dataCanvas->Update();
  }

  // 3-d plot of the parameter points
  dataCanvas->cd(2);
  // also plot the points in the markov chain
  RooDataSet* markovChainData = ((MCMCInterval*)mcmcint)->GetChainAsDataSet();

  TTree& chain =  ((RooTreeDataStore*) markovChainData->store())->tree();
  chain.SetMarkerStyle(6);
  chain.SetMarkerColor(kRed);
  chain.Draw("s:ratioSigEff:ratioBkgEff","","box"); // 3-d box proporional to posterior

  // the points used in the profile construction
  TTree& parameterScan =  ((RooTreeDataStore*) fc.GetPointsToScan()->store())->tree();
  parameterScan.SetMarkerStyle(24);
  parameterScan.Draw("s:ratioSigEff:ratioBkgEff","","same");

  chain.SetMarkerStyle(6);
  chain.SetMarkerColor(kRed);
  //chain.Draw("s:ratioSigEff:ratioBkgEff", "_MarkovChain_local_nll","box");
  //chain.Draw("_MarkovChain_local_nll");

  TCanvas* walkCanvas = new TCanvas("walkCanvas");
  walkCanvas->cd();
  mcPlot.DrawChainScatter(*s, *ratioBkgEff);

  TCanvas* timeCanvas = new TCanvas("timeCanvas");
  timeCanvas->Divide(2);
  timeCanvas->cd(1);
  mcPlot.DrawParameterVsTime(*s);
  timeCanvas->cd(2);
  mcPlot.DrawNLLVsTime();
  timeCanvas->Update();

  TCanvas* nllHistCanvas = new TCanvas("nllHistCanvas");
  mcPlot.DrawNLLHist();
  nllHistCanvas->Update();

  TCanvas* weightHistCanvas = new TCanvas("weightHistCanvas");
  mcPlot.DrawWeightHist();
  weightHistCanvas->Update();

  TCanvas* productCanvas = new TCanvas("productCanvas");
  mcPlot.DrawPosteriorKeysProduct();
  //mcPlot.DrawPosteriorKeysPdf("same"); // not normalized correctly
  productCanvas->Update();

  TFile chainDataFile("chainData_101_limitexample.root", "recreate");
  markovChainData->Write();
  chainDataFile.Close();

  delete wspace;
  delete lrint;
  delete fcint;
  delete mcmcint;
  delete data;

  /// print timing info
  t.Stop();
  t.Print();
  printf("hist interval in 2-wide orange, keyspdf interval in 2-wide magenta\n");
}
