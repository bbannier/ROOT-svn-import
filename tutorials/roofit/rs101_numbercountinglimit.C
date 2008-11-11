/////////////////////////////////////////////////////////////////////////
//
// 'Number Counting Example' RooStats tutorial macro #100
// 
//
// 11/2008 - Kyle Cranmer
//
/////////////////////////////////////////////////////////////////////////

#ifndef __CINT__
#include "RooGlobalFunc.h"
#endif
#include "RooStats/ProfileLikelihoodCalculator.h"
#include "RooStats/NumberCountingPdfFactory.h"
#include "RooStats/LikelihoodInterval.h"
#include "RooRealVar.h"
#include "RooNllVar.h"
#include "RooProfileLL.h"

using namespace RooFit ;
using namespace RooStats ;


void rs101_numbercountinglimit()
{

  // Step 1, define arrays with signal & bkg expectations and background uncertainties
  Double_t s[2] = {10.,10.};
  Double_t b[2] = {100.,100.};
  Double_t db[2] = {.0100,.0100};
  
  // Step 2, use a RooStats factory to build a PDF for a number counting combination
  NumberCountingPdfFactory f;
  RooWorkspace* ws2 = f.GetExpWS(s,b,db,2); // see below for a printout of the workspace


  RooRealVar* param = (RooRealVar*) ws2->var("masterSignal"); 
  RooArgSet* params = new RooArgSet("params");
  RooAbsData* data =  ws2->data("ExpectedNumberCountingData"); 
  params->addClone(*param);

  RooNLLVar nll("nll","",*(ws2->pdf("joint")),*data);
  RooProfileLL profile("pll","",nll,*(ws2->var("masterSignal")));

  LikelihoodInterval lrint("LRInterval","",&profile);

  params->setRealValue("masterSignal",0); 
  std::cout << "\nconsider this parameter point:" << std::endl;
  params->first()->Print();
  std::cout << "\tis it in the interval?: " << lrint.IsInInterval(*params) << std::endl;

  params->setRealValue("masterSignal",1); 
  std::cout << "\nconsider this parameter point:" << std::endl;
  params->first()->Print();
  std::cout << "\tis it in the interval?: " << lrint.IsInInterval(*params) << std::endl;

  params->setRealValue("masterSignal",3); 
  std::cout << "\nconsider this parameter point:" << std::endl;
  params->first()->Print();
  cout << "\tis it in the interval?: " << lrint.IsInInterval(*params) << std::endl;
  
}
