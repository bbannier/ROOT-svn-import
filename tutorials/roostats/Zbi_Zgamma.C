/////////////////////////////////////////////////////////////////////////
//
// Demonstraite Z_Bi = Z_Gamma
// author: Kyle Cranmer & Wouter Verkerke
// date May 2010
//
//
/////////////////////////////////////////////////////////////////////////

#ifndef __CINT__
#include "RooGlobalFunc.h"
#endif
#include "RooRealVar.h"
#include "RooProdPdf.h"
#include "RooWorkspace.h"
#include "RooDataSet.h"
#include "TCanvas.h"
#include "TH1.h"

using namespace RooFit;
using namespace RooStats;

void Zbi_Zgamma() {

  RooWorkspace* w = new RooWorkspace("w",true);
  RooAbsPdf* px = (RooAbsPdf*)   w->factory("Poisson::px(x[0,500],sum::splusb(s[0,0,100],b[100,0,300]))");
  RooAbsPdf* py = (RooAbsPdf*)   w->factory("Poisson::py(y[100,0,500],prod::taub(tau[1.],b))");	     
  RooRealVar* x = w->var("x");
  RooRealVar* y =  w->var("y");
  RooRealVar* b =  w->var("b");
  RooAbsPdf* pdf = (RooAbsPdf*) w->factory("PROD::pdf(px,py)");
  RooAbsPdf* prior = (RooAbsPdf*) w->factory("Uniform::prior_b(b)");

  // construct the Bayesian averaged model:
  // p'(x|s) = \int db p(x|s+b) * [ p(y|b) * prior(b) ]
  w->factory("PROJ::ifoo(PROD::foo(px|b,py,prior_b),b)") ;

  // plot it, blue is smeared model, red is b known exactly
  RooPlot* frame = w->var("x")->frame() ;
  w->pdf("ifoo")->plotOn(frame) ;
  w->pdf("px")->plotOn(frame,LineColor(kRed)) ;
  frame->Draw() ;


  // compare Z_Bi analytic calculation with the numerical
  // RooFit implementation of Z_Gamma
  // for an example with x = 144, y = 100

   
  // numeric RooFit Z_Gamma
  w->var("y")->setVal(100);
  w->var("x")->setVal(144);
  RooAbsReal* cdf = w->pdf("ifoo")->createCdf(*w->var("x"));
  cdf->getVal(); // get ugly print messages out of the way

  cout << "Hybrid p-value = " << cdf->getVal() << endl;
  cout << "Z_Gamma Significance  = " << 
    RooStats::PValueToSignificance(1-cdf->getVal()) << endl;

  // analytic Z_Bi
  double Z_Bi = RooStats::NumberCountingUtils::BinomialWithTauObsZ(144, 100, 1);
  std::cout << "Z_Bi significance estimation: " << Z_Bi << std::endl;

  // OUTPUT
  //Hybrid p-value = 0.997101
  //Z_Gamma Significance  = 2.75905
  //Z_Bi significance estimation: 2.75905


}
