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
#include "RooStats/FeldmanCousins.h"
#include "RooStats/NumberCountingPdfFactory.h"
#include "RooStats/ConfInterval.h"
#include "RooStats/LikelihoodInterval.h"
#include "RooProfileLL.h"
#include "RooAbsPdf.h"
#include "RooStats/HypoTestResult.h"
#include "RooRealVar.h"
#include "RooPlot.h"
#include "RooDataSet.h"

#include "RooStats/RooStatsUtils.h"

// use this order for safety on library loading
using namespace RooFit ;
using namespace RooStats ;


void rs101_limitexample()
{
  /////////////////////////////////////////
  // An example of setting a limit in a number counting experiment with uncertainty on background and signal
  /////////////////////////////////////////

  /////////////////////////////////////////
  // The Model building stage
  /////////////////////////////////////////
  RooWorkspace* wspace = new RooWorkspace();
  wspace->factory("Poisson::countingModel(obs[150,0,300], sum(s[50,0,100]*ratioSigEff[1.,0,2.],b[100,0,300]*ratioBkgEff[1.,0.,2.]))"); // counting model
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

  // a toy dataset
  RooDataSet* data = modelWithConstraints->generate(*obs, 1);
  cout << "----------" << endl;

  // make a plot of the -log likelihood ratio and -log profile likelihood ratio
  RooPlot* frame = s->frame();
  RooAbsReal* nll = modelWithConstraints->createNLL(*data, Constrain(constrainedParams) );
  RooAbsReal* profile = nll->createProfile(*s);
  nll->plotOn(frame, LineColor(kRed), ShiftToZero());
  profile->plotOn(frame);
  frame->Draw();

  cout << "----------" << endl;


  // XXXXXXXXXXXXXXXXXXXXXXXXXXXXXX
  // temp for debugging
  //  ratioSigEff->setConstant();
  //  ratioBkgEff->setConstant();
  // XXXXXXXXXXXXXXXXXXXXXXXXXXXXXX

  // Now let's make some confidence intervals for s, our parameter of interest
  RooArgSet paramOfInterest(*s);


  cout << "----------" << endl;

  // First, let's use a Calculator based on the Profile Likelihood Ratio
  ProfileLikelihoodCalculator plc;
  //  plc.SetWorkspace(*wspace);
  plc.SetPdf(*modelWithConstraints);
  plc.SetData(*data); 
  plc.SetParameters( paramOfInterest );
  plc.SetTestSize(.1);
  ConfInterval* lrint = plc.GetInterval();  // that was easy.

  cout << "----------" << endl;

  // Second, use a Calculator based on the Feldman Cousins technique
  FeldmanCousins fc;
  //  fc.SetWorkspace(*wspace);
  fc.SetPdf(*modelWithConstraints);
  fc.SetData(*data); 
  fc.SetParameters( paramOfInterest );
  fc.UseAdaptiveSampling(true);
  fc.FluctuateNumDataEntries(false); // number counting analysis: dataset always has 1 entry with N events observed
  fc.SetNBins(3); // number of points to test per parameter
  fc.SetTestSize(.1);
  ConfInterval* fcint = 0;
  fcint = fc.GetInterval();  // that was easy.


  s->setVal(55);
  if( lrint->IsInInterval(paramOfInterest) ) 
    cout << "s = " << s->getVal() << " is in interval" << endl;
  else
    cout << "s = " << s->getVal() << " is NOT in interval" << endl;

  // Now let's check some specific points to see if they are in the interval

  s->setVal(55);
  if( lrint->IsInInterval(paramOfInterest) ) 
    cout << "s = " << s->getVal() << " is in interval" << endl;
  else
    cout << "s = " << s->getVal() << " is NOT in interval" << endl;

  if(fcint){
    if( fcint->IsInInterval(paramOfInterest) ) 
      cout << "s = " << s->getVal() << " is in FC interval" << endl;
    else
      cout << "s = " << s->getVal() << " is NOT in FC interval" << endl;
  }


  // if fcint is not commented out the profile in the lrint doesn't work properly
  ratioBkgEff->setVal(1.33);
  ratioSigEff->setVal(1.06);
  s->setVal(55);
  if( lrint->IsInInterval(paramOfInterest) ) 
    cout << "s = " << s->getVal() << " is in interval" << endl;
  else
    cout << "s = " << s->getVal() << " is NOT in interval" << endl;

  if(fcint){
    if( fcint->IsInInterval(paramOfInterest) ) 
      cout << "s = " << s->getVal() << " is in FC interval" << endl;
    else
      cout << "s = " << s->getVal() << " is NOT in FC interval" << endl;
  }
  
  
  s->setVal(20);
  if( lrint->IsInInterval(paramOfInterest) ) 
    cout << "s = " << s->getVal() << " is in interval" << endl;
  else
    cout << "s = " << s->getVal() << " is NOT in interval" << endl;

  s->setVal(80);
  if( lrint->IsInInterval(paramOfInterest) ) 
    cout << "s = " << s->getVal() << " is in interval" << endl;
  else
    cout << "s = " << s->getVal() << " is NOT in interval" << endl;

  //  cout << "ll = " << ((LikelihoodInterval*) lrint)->LowerLimit(*s) << endl;
  //  cout << "ul = " << ((LikelihoodInterval*) lrint)->UpperLimit(*s) << endl;
  // now use a tool

  delete wspace;
  delete lrint;
  delete fcint;
  delete data;
  //  delete frame;
}
