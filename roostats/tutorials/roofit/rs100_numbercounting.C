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
#include "RooStats/ConfInterval.h"
#include "RooRealVar.h"

using namespace RooFit ;
using namespace RooStats ;


void rs100_numbercounting()
{

  // Step 1, define arrays with signal & bkg expectations and background uncertainties
  Double_t s[2] = {10.,10.};
  Double_t b[2] = {100.,100.};
  Double_t db[2] = {.0100,.0100};
  
  // Step 2, use a RooStats factory to build a PDF for a number counting combination
  NumberCountingPdfFactory f;
  RooWorkspace* ws2 = f.GetExpWS(s,b,db,2); // see below for a printout of the workspace

  // Step 3, Create a calculator for doing the hypothesis test.
  ProfileLikelihoodCalculator plc;
  plc.SetWorkspace(ws2);
  plc.SetCommonPdf("joint");
  plc.SetData("ExpectedNumberCountingData");

  // Step 4, Define the null hypothesis for the calculator
  // here you need to know the name of the variables corresponding to hypothesis.
  RooRealVar* x = ws2->var("masterSignal"); 
  RooArgSet* nullParams = new RooArgSet("nullParams");
  nullParams->addClone(*x);
  // here we explicitly set the value of the parameters for the null
  nullParams->setRealValue("masterSignal",0); 
  plc.SetNullParameters(nullParams);

  // Step 5, Use the Calculator to get a HypoTestResult
  HypoTestResult* htr = plc.GetHypoTest();
  cout << "The p-value for the null is " << htr->NullPValue() << endl;
  cout << "Corresponding to a signifcance of " << htr->Significance() << endl;

  /* should return:
    The p-value for the null is 0.0831114
    Corresponding to a signifcance of 1.38444
  */

  delete htr;


  // an example with the confidence interval
  /*
  plc.SetParameters(nullParams);
  ConfInterval* lrint = plc.GetInterval();
  nullParams->setRealValue("masterSignal",2.); 
  std::cout << "\nconsider this parameter point:" << std::endl;
  nullParams->first()->Print();
  std::cout << "\tis it in the interval?: " << lrint->IsInInterval(*nullParams) << std::endl;

  delete lrint;
  */


  //  delete nullParams;

  /*
  // Here's an example of what is in the workspace
  //  ws2->Print();
  RooWorkspace(NumberCountingWS) Number Counting WS contents

  variables
  ---------
  (x_0,masterSignal,expected_s_0,b_0,y_0,tau_0,x_1,expected_s_1,b_1,y_1,tau_1)
  
  p.d.f.s
  -------
  RooProdPdf::joint[ pdfs=(sigRegion_0,sideband_0,sigRegion_1,sideband_1) ] = 2.29923e-08
  RooPoisson::sigRegion_0[ x=x_0 mean=splusb_0 ] = 0.0380088
  RooPoisson::sideband_0[ x=y_0 mean=bTau_0 ] = 0.00398939
  RooPoisson::sigRegion_1[ x=x_1 mean=splusb_1 ] = 0.0380088
  RooPoisson::sideband_1[ x=y_1 mean=bTau_1 ] = 0.00398939
  
  functions
  --------
  RooAddition::splusb_0[ set1=(s_0,b_0) set2=() ] = 110
  RooProduct::s_0[ compRSet=(masterSignal,expected_s_0) compCSet=() ] = 10
  RooProduct::bTau_0[ compRSet=(b_0,tau_0) compCSet=() ] = 10000
  RooAddition::splusb_1[ set1=(s_1,b_1) set2=() ] = 110
  RooProduct::s_1[ compRSet=(masterSignal,expected_s_1) compCSet=() ] = 10
  RooProduct::bTau_1[ compRSet=(b_1,tau_1) compCSet=() ] = 10000

  datasets
  --------
  RooDataSet::NumberCountingData(x_0,y_0,x_1,y_1)
  */
  
}
