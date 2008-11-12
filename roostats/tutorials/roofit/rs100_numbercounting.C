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

  /////////////////////////////////////////
  // An example of a number counting combination with two channels.
  // We consider both hypothesis testing and the equivalent confidence interval.
  // Note: Here we consider the case of an expected result. 
  // Will update tutorial to consider observed data
  /////////////////////////////////////////


  /////////////////////////////////////////
  // The Model building stage
  /////////////////////////////////////////

  // Step 1, define arrays with signal & bkg expectations and background uncertainties
  Double_t s[2] = {20.,10.};      // expected signal
  Double_t b[2] = {100.,100.};    // expected background
  Double_t db[2] = {.0100,.0100}; // fractional background uncertainty

  
  // Step 2, use a RooStats factory to build a PDF for a number counting combination
  // Workspace will include model and expected data.
  // Could add observed data to the workspace at this stage.
  // The model neglects correlations in background uncertainty, but they can be added.
  NumberCountingPdfFactory f;
  RooWorkspace* ws2 = f.GetExpWS(s,b,db,2); // see below for a printout of the workspace
  //  ws2->Print();  //uncomment to see structure of workspace

  // Step 2b.
  Double_t mainMeas[2] = {125.,115.};      // observed main measurement
  Double_t sideband[2] = {10000.,10000.};    // observed sideband
  f.AddObsData(mainMeas,sideband,2,ws2);

  /////////////////////////////////////////
  // The Hypothesis testing stage:
  /////////////////////////////////////////

  // Step 3, Create a calculator for doing the hypothesis test.
  ProfileLikelihoodCalculator plc;
  plc.SetWorkspace(ws2);
  plc.SetCommonPdf("joint");
  plc.SetData("ExpectedNumberCountingData");
  //  plc.SetData("ObservedNumberCountingData");

  // Step 4, Define the null hypothesis for the calculator
  // Here you need to know the name of the variables corresponding to hypothesis.
  RooRealVar* x = ws2->var("masterSignal"); 
  RooArgSet* nullParams = new RooArgSet("nullParams");
  nullParams->addClone(*x);
  // here we explicitly set the value of the parameters for the null
  nullParams->setRealValue("masterSignal",0); 
  plc.SetNullParameters(nullParams);

  // Step 5, Use the Calculator to get a HypoTestResult
  HypoTestResult* htr = plc.GetHypoTest();
  cout << "-------------------------------------------------" << endl;
  cout << "The p-value for the null is " << htr->NullPValue() << endl;
  cout << "Corresponding to a signifcance of " << htr->Significance() << endl;
  cout << "-------------------------------------------------\n\n" << endl;

  /* should return:
     The p-value for the null is 0.015294
     Corresponding to a signifcance of 2.16239
  */

  delete htr;

  //////////////////////////////////////////
  // Confidence Interval Stage

  // Step 5, Here we re-use the ProfileLikelihoodCalculator to return a confidence interval.
  // We need to specify what are our parameters of interest
  RooArgSet* paramsOfInterest = nullParams; // they are the same as before in this case
  plc.SetParameters(paramsOfInterest);
  ConfInterval* lrint = plc.GetInterval();  // that was easy.
  lrint->SetConfidenceLevel(0.95);

  // Step 6, Ask if masterSignal=0 is in the interval.
  // Note, this is equivalent to the question of a 2-sigma hypothesis test: 
  // "is the parameter point masterSignal=0 inside the 95% confidence interval?"
  // Since the signficance of the Hypothesis test was > 2-sigma it should not be: 
  // eg. we exclude masterSignal=0 at 95% confidence.
  paramsOfInterest->setRealValue("masterSignal",0.); 
  cout << "-------------------------------------------------" << endl;
  std::cout << "Consider this parameter point:" << std::endl;
  paramsOfInterest->first()->Print();
  if( lrint->IsInInterval(*paramsOfInterest) )
    std::cout << "It IS in the interval."  << std::endl;
  else
    std::cout << "It is NOT in the interval."  << std::endl;
  cout << "-------------------------------------------------\n\n" << endl;

  // We also ask about the parameter point masterSignal=2, which is inside the interval.
  paramsOfInterest->setRealValue("masterSignal",2.); 
  cout << "-------------------------------------------------" << endl;
  std::cout << "Consider this parameter point:" << std::endl;
  paramsOfInterest->first()->Print();
  if( lrint->IsInInterval(*paramsOfInterest) )
    std::cout << "It IS in the interval."  << std::endl;
  else
    std::cout << "It is NOT in the interval."  << std::endl;
  cout << "-------------------------------------------------\n\n" << endl;
  
  delete lrint;
  //  delete ws2;
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
  RooProdPdf::joint[ pdfs=(sigRegion_0,sideband_0,sigRegion_1,sideband_1) ] = 2.20148e-08
  RooPoisson::sigRegion_0[ x=x_0 mean=splusb_0 ] = 0.036393
  RooPoisson::sideband_0[ x=y_0 mean=bTau_0 ] = 0.00398939
  RooPoisson::sigRegion_1[ x=x_1 mean=splusb_1 ] = 0.0380088
  RooPoisson::sideband_1[ x=y_1 mean=bTau_1 ] = 0.00398939
  
  functions
  --------
  RooAddition::splusb_0[ set1=(s_0,b_0) set2=() ] = 120
  RooProduct::s_0[ compRSet=(masterSignal,expected_s_0) compCSet=() ] = 20
  RooProduct::bTau_0[ compRSet=(b_0,tau_0) compCSet=() ] = 10000
  RooAddition::splusb_1[ set1=(s_1,b_1) set2=() ] = 110
  RooProduct::s_1[ compRSet=(masterSignal,expected_s_1) compCSet=() ] = 10
  RooProduct::bTau_1[ compRSet=(b_1,tau_1) compCSet=() ] = 10000
  
  datasets
  --------
  RooDataSet::ExpectedNumberCountingData(x_0,y_0,x_1,y_1)
  
  embedded precalculated expensive components
  -------------------------------------------
  */
  
}
