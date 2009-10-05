/////////////////////////////////////////////////////////////////////////
//
// 'Bayesian Calculator' RooStats tutorial macro #701
// author: Gregory Schott
// date Sep 2009
//
// This tutorial shows an example of using the BayesianCalculator class 
//
/////////////////////////////////////////////////////////////////////////

#include "RooRealVar.h"
#include "RooProdPdf.h"
#include "RooWorkspace.h"
#include "RooDataSet.h"

#include "RooStats/BayesianCalculator.h"
#include "RooStats/SimpleInterval.h"

#include "TCanvas.h"
#include "TPad.h"
#include "TAxis.h"
#include "TVirtualPad.h"
#include "RooPlot.h"

using namespace RooFit;
using namespace RooStats;

void rs702_BayesianCalculator()
{
  // using namespace RooFit;
  // using namespace RooStats;

  RooWorkspace* w = new RooWorkspace("w",true);
  w->factory("SUM::pdf(s[1.01,10]*Uniform(x[0,1]),b[1,0,3]*Uniform(x))");
  w->factory("Gaussian::prior_b(b,1,0.8)");
  w->factory("PROD::model(pdf,prior_b)");
  RooAbsPdf* model = w->pdf("model");
  RooArgSet* nuisanceParameters = new RooArgSet(*(w->var("b")));
  RooArgSet* noNuisanceParameters = new RooArgSet();

  w->factory("Uniform::priorPOI(s)");
  w->factory("GenericPdf::priorPOI2('1/s',s)");
  RooRealVar* POI = w->var("s");
  RooAbsPdf* priorPOI = w->pdf("priorPOI");
  RooAbsPdf* priorPOI2 = w->pdf("priorPOI2");
  
  w->factory("n[2]");  // number of observed events
  RooDataSet data("data","",RooArgSet(*(w->var("x")),*(w->var("n"))),"n");
  data.add(RooArgSet(*(w->var("x"))),w->var("n")->getVal());

  BayesianCalculator bcalc1(data,*model,RooArgSet(*POI),*priorPOI,nuisanceParameters);
  BayesianCalculator bcalc2(data,*model,RooArgSet(*POI),*priorPOI2,nuisanceParameters);
  BayesianCalculator bcalc3(data,*(w->pdf("pdf")),RooArgSet(*POI),*priorPOI,noNuisanceParameters);
  bcalc1.SetTestSize(0.05);
  bcalc2.SetTestSize(0.05);
  bcalc3.SetTestSize(0.05);

  SimpleInterval* interval1 = bcalc1.GetInterval();
  SimpleInterval* interval2 = bcalc2.GetInterval();
  SimpleInterval* interval3 = bcalc3.GetInterval();
  std::cout << "90% CL interval: [ " << interval1->LowerLimit() << " - " << interval1->UpperLimit() << " ] or 95% CL limits\n";
  std::cout << "90% CL interval: [ " << interval2->LowerLimit() << " - " << interval2->UpperLimit() << " ] or 95% CL limits\n";
  std::cout << "90% CL interval: [ " << interval3->LowerLimit() << " - " << interval3->UpperLimit() << " ] or 95% CL limits\n";

  TCanvas* c1 = new TCanvas();
  c1->Divide(1,2);
  TPad* pad = (TPad *) c1->cd(1);
  pad->Divide(2,1);
  pad->cd(1);
  RooPlot* plotPrior = POI->frame();
  plotPrior->SetTitle(TString("Prior probability of parameter \"")+TString(POI->GetName())+TString("\""));  
  priorPOI->plotOn(plotPrior);
  priorPOI2->plotOn(plotPrior,LineStyle(kDashed),LineColor(kGreen));
  plotPrior->Draw();

  pad->cd(2);
  RooPlot* plotSyst = w->var("b")->frame();
  plotSyst->SetTitle(TString("Prior on nuisance parameter \"")+TString(POI->GetName())+TString("\""));  
  w->pdf("prior_b")->plotOn(plotSyst,LineStyle(kDashed),LineColor(kRed));   
  plotSyst->Draw();

  c1->cd(2);

  RooAbsPdf* fPosteriorPdf1 = bcalc1.GetPosteriorPdf();
  RooAbsPdf* fPosteriorPdf2 = bcalc2.GetPosteriorPdf();
  RooAbsPdf* fPosteriorPdf3 = bcalc3.GetPosteriorPdf();

  RooPlot* plot = POI->frame();
  plot->SetTitle(TString("Posterior probability of parameter \"")+TString(POI->GetName())+TString("\""));  
  fPosteriorPdf1->plotOn(plot);
  fPosteriorPdf2->plotOn(plot,LineStyle(kDashed),LineColor(kGreen));
  fPosteriorPdf3->plotOn(plot,LineStyle(kDashed),LineColor(kRed));
  plot->GetYaxis()->SetTitle("posterior probability");
  plot->Draw();

  // plot of cdf
//   RooAbsReal * cdf1 =  fPosteriorPdf1->createCdf(RooArgSet(*POI));
//   RooAbsReal * cdf2 =  fPosteriorPdf2->createCdf(RooArgSet(*POI));
//   RooPlot* plot2 = POI->frame();
//   cdf1->plotOn(plot2);
//   cdf2->plotOn(plot2,LineStyle(kDashed),LineColor(kGreen));
//   plot2->Draw();

  c1->cd();
}
