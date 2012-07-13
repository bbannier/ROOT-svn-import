
#include <iostream>
#include <string>
#include "test.h"

#ifndef __CINT__
#include "RooGlobalFunc.h"
#endif
#include "TRandom.h"
#include "TBenchmark.h"
#include "RooRealVar.h"
#include "RooBernstein.h"
#include "TMath.h"
#include "RooDataHist.h"
#include "RooCategory.h"
#include "RooPoisson.h"
#include "RooGaussian.h"
#include "RooUniform.h"
#include "RooExponential.h"
#include "RooPlot.h"
#include "TCanvas.h"
#include "RooRandom.h"
#include "RooDataSet.h"
#include "RooStats/RooStatsUtils.h"
#include "TSystem.h"

using namespace RooFit;

void test() {

   gSystem->Load("libRooFit");
   RooWorkspace *w = new RooWorkspace("w", true);

   w->factory("Gaussian::constrb(b0[-5,5],b1[-5,5],1)");
   w->factory("Gaussian::constre(e0[-5,5],e1[-5,5],1)");
   w->factory("expr::bkg('5 * pow(1.3, b1)', b1)");
   w->factory("expr::eff('0.5 * pow(1.2, e1)', e1)");
   w->factory("expr::esb('eff * sig + bkg', eff, bkg, sig[0,50])");
   w->factory("Poisson::poiss(x[0,50],esb)");
   w->factory("PROD::pdf(poiss,constrb,constre)");
   w->defineSet("obs","x");

   RooArgList constraints;
   RooArgList obsTerms;


//   RooStats::Utils::FactorizePdf(*w->var("x"), *w->pdf("pdf"), constraints)->Print("v");
   RooStats::RemoveNuisancePdf(*w->pdf("pdf"), *w->set("obs"), "blablabla")->Print();
   

}


int main() {
   test();
   return 0;
}
