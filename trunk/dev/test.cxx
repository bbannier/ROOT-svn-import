#ifndef __CINT__
#include "RooGlobalFunc.h"
#endif

#include "TSystem.h"
#include "TClassTable.h"
#include "TClass.h"
#include "TROOT.h"
#include "RooRealVar.h"
#include "RooDataSet.h"
#include "RooAbsData.h"
#include "RooGaussian.h"
#include "RooWorkspace.h"
#include "RooStats/ModelConfig.h"
#include "RooStats/ProfileLikelihoodCalculator.h"
#include "RooStats/LikelihoodInterval.h"
#include "RooStats/LikelihoodIntervalPlot.h"
#include "RooFactoryWSTool.h"

using namespace RooFit;
using namespace RooStats;

int main(int argc, char *argv[]) {

   // WARNING: classes are not loaded into ROOT table even if libRooFit is linked via Makefile
// gSystem->Load("libRooFit");

   RooWorkspace* w = new RooWorkspace("w", kTRUE);

   w->factory("Gaussian::g(x[-10,10], mean[0,-10,10], sigma[5,0,10])");
   RooAbsPdf *g = w->pdf("g");
   RooRealVar *x = w->var("x");
   RooRealVar *mean = w->var("mean");
   RooRealVar *sigma = w->var("sigma");

   RooDataSet *data = g->generate(*x, 10000);
   RooArgSet *params = new RooArgSet(); params->add(*mean);
   
   // QUESTION: any difference? Why sigma as well?
// params->add(*sigma);

   // WARNING: this loads the classes
// RooAbsPdf *g = new RooGaussian();
// RooRealVar *x = new RooRealVar();
// RooRealVar *mean = new RooRealVar();



   cout << "Some value " << g->getVal(*x) << endl;
   cout << "Unnormalized" << g->getVal() << endl;


   // WARNING: this also loads RooGaussian somehow -> weird
   ProfileLikelihoodCalculator *pl = new ProfileLikelihoodCalculator(*data,*g, *params);
   pl->SetConfidenceLevel(0.95);
      
   LikelihoodInterval *interval = pl->GetInterval();
   
   cout << "\n95\% interval on " << mean->GetName() << " is :: [" <<
      interval->LowerLimit(*mean) << ", " << interval->UpperLimit(*mean) << "] " << endl;



   cout << "Making a plot of the profile likelihood function ... (if it is taking a lot of time use less points or the TF1 drawing option)\n";
   LikelihoodIntervalPlot *plot = new LikelihoodIntervalPlot(interval);
   plot->SetNPoints(50);
   plot->Draw(""); // use option TF1 if too slow


   delete data;
   delete params;
   delete pl;
   delete interval;
   delete plot;
   delete w; 
}


