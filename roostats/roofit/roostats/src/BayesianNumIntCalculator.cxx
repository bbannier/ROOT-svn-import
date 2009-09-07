/**
   BayesianNumIntCalculator class
**/

// include other header files

#include "RooAbsFunc.h"
#include "RooAbsReal.h"
#include "RooAbsRealLValue.h"
#include "RooArgSet.h"
#include "RooBrentRootFinder.h"
#include "RooFormulaVar.h"
#include "RooGenericPdf.h"
#include "RooPlot.h"
#include "RooProdPdf.h"

// include header file of this class 
#include "RooStats/BayesianNumIntCalculator.h"

ClassImp(RooStats::BayesianNumIntCalculator)

using namespace RooStats;


BayesianNumIntCalculator::BayesianNumIntCalculator( const char* name,
						    const char* title ) :
  TNamed( TString(name), TString(title) )
{
  // default constructor
}


BayesianNumIntCalculator::BayesianNumIntCalculator( const char* name,
						    const char* title,
						    RooAbsPdf* pdf,
						    RooAbsData* data,
						    RooAbsRealLValue* POI,
						    RooAbsPdf* priorPOI,
						    RooArgSet* nuisanceParameters ) :
  TNamed( TString(name), TString(title) ),
  fPdf(pdf),
  fData(data),
  fPOI(POI),
  fPriorPOI(priorPOI),
  fNuisanceParameters(nuisanceParameters)
{
  // constructor
  fLowerLimit = -999;
  fUpperLimit = +999;
}


BayesianNumIntCalculator::~BayesianNumIntCalculator()
{
  // destructor
}


RooPlot* BayesianNumIntCalculator::PlotPosterior()
{
  RooProdPdf posterior("posterior","",RooArgList(*fPdf,*fPriorPOI));
  RooAbsReal* nll = posterior.createNLL(*fData);
  RooFormulaVar like("like","exp(-@0)",RooArgList(*nll));

  RooPlot* plot = fPOI->frame();
  like.plotOn(plot);
  //like.plotOn(plot,RooFit::Range(fLowerLimit,fUpperLimit),RooFit::FillColor(kYellow));
  //plot->GetYaxis()->SetTitle("posterior probability");
  plot->Draw();

  delete nll;
  return plot; 
}


SimpleInterval* BayesianNumIntCalculator::GetInterval()
{
  // returns a SimpleInterval with the lower/upper limit on the scanned variable

  RooProdPdf posterior("posterior","",RooArgList(*fPdf,*fPriorPOI));
  RooAbsReal* nll = posterior.createNLL(*fData);
  RooFormulaVar like("like","exp(-@0)",RooArgList(*nll));

  RooGenericPdf pp("pp","@0",like);
  RooAbsReal* cdf = pp.createCdf(RooArgSet(*fPOI,*fNuisanceParameters));
    
  RooArgSet* nset = new RooArgSet(*fPOI);
  RooAbsFunc* cdf_bind = cdf->bindVars(*nset,nset);
  RooBrentRootFinder brf(*cdf_bind);
  brf.setTol(0.00005);

  double y(0.05);
  brf.findRoot(fLowerLimit,fPOI->getMin(),fPOI->getMax(),y);

  y=0.95;
  brf.findRoot(fUpperLimit,fPOI->getMin(),fPOI->getMax(),y);

  delete cdf_bind;
  delete nset;
  delete cdf;
  delete nll;

  TString interval_name = this->GetName();
  interval_name += "_interval";
  SimpleInterval* interval = new SimpleInterval(interval_name,"SimpleInterval from BayesianNumIntCalculator",fPOI,fLowerLimit,fUpperLimit);
  return interval;
}


