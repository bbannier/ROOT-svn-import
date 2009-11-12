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

using namespace RooFit;
using namespace RooStats;

void rs701_BayesianCalculator()
{

  RooWorkspace* w = new RooWorkspace("w",true);
  w->factory("SUM::pdf(s[0.001,10]*Uniform(x[0,1]),b[0.001,0,2]*Uniform(x))");
  w->factory("Gaussian::prior_b(b,0.001,1)");
  w->factory("PROD::model(pdf,prior_b)");
  RooAbsPdf* model = w->pdf("model");  // pdf*priorNuisance
  const RooArgSet nuisanceParameters(*(w->var("b")));

  w->factory("Uniform::priorPOI(s)");
  RooRealVar* POI = w->var("s");
  RooAbsPdf* priorPOI = w->pdf("priorPOI");

  w->factory("n[2]"); // observed number of events
  RooDataSet data("data","",RooArgSet(*(w->var("x")),*(w->var("n"))),"n");
  data.add(RooArgSet(*(w->var("x"))),w->var("n")->getVal());

  BayesianCalculator bcalc(data,*model,RooArgSet(*POI),*priorPOI,&nuisanceParameters);
  bcalc.SetTestSize(0.05);

  SimpleInterval* interval = bcalc.GetInterval();
  std::cout << "90% CL interval: [ " << interval->LowerLimit() << " - " << interval->UpperLimit() << " ] or 95% CL limits\n";

  // observe one event while expecting one background event -> the 95% CL upper limit on s is 4.10
  // observe one event while expecting zero background event -> the 95% CL upper limit on s is 4.74

  // The plotting code below can be replace by the single following line in the ROOT > 5.25.00 future releases
  //bcalc.GetPosteriorPlot()->Draw();

  // bcalc.PlotPosterior(); // for the moment this is the current plot

  // the commented code below produce a much nicer plot

  RooAbsPdf* fPosteriorPdf = bcalc.GetPosteriorPdf();

  RooPlot* plot = POI->frame();
  plot->SetTitle(TString("Posterior probability of parameter \"")+TString(POI->GetName())+TString("\""));  
  fPosteriorPdf->plotOn(plot,RooFit::Range(interval->LowerLimit(),interval->UpperLimit(),kFALSE),RooFit::VLines(),RooFit::DrawOption("F"),RooFit::MoveToBack(),RooFit::FillColor(kGray));
  fPosteriorPdf->plotOn(plot);
  plot->GetYaxis()->SetTitle("posterior probability");
  plot->Draw();
  
}
