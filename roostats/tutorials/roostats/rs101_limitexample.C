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
#include "RooStats/NumberCountingPdfFactory.h"
#include "RooStats/ConfInterval.h"
#include "RooStats/LikelihoodInterval.h"
#include "RooProfileLL.h"
#include "RooAbsPdf.h"
#include "RooStats/HypoTestResult.h"
#include "RooRealVar.h"
#include "RooPlot.h"
#include "RooDataSet.h"

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
  wspace->factory("Poisson::countingModel(obs[150,0,300], sum::tot(prod::sig(s[50,0,300],ratioSigEff[1.,0,2.]),prod::bkg(b[100,0,300],ratioBkgEff[1.,0.,2.])))"); // counting model
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

  

  modelWithConstraints->fitTo(*data, Constrain(constrainedParams) );

  RooPlot* frame = s->frame();
  RooAbsReal* nll = modelWithConstraints->createNLL(*data, Constrain(constrainedParams) );
  RooAbsReal* profile = nll->createProfile(*s);
  nll->plotOn(frame, LineColor(kRed), ShiftToZero());
  profile->plotOn(frame);
  frame->Draw();



  RooArgSet paramOfInterest(*s);
  paramOfInterest.Print("v");
  ProfileLikelihoodCalculator plc;
  plc.SetWorkspace(*wspace);
  plc.SetCommonPdf(*modelWithConstraints);
  plc.SetData(*data); 
  plc.SetParameters( paramOfInterest );
  plc.SetTestSize(.05);

  ConfInterval* lrint = plc.GetInterval();  // that was easy.
  //  lrint->SetConfidenceLevel(0.95);
  if( lrint->IsInInterval(paramOfInterest) ) 
    cout << "s = " << s->getVal() << " is in interval" << endl;
  else
    cout << "s = " << s->getVal() << " is NOT in interval" << endl;

  s->setVal(0);
  if( lrint->IsInInterval(paramOfInterest) ) 
    cout << "s = " << s->getVal() << " is in interval" << endl;
  else
    cout << "s = " << s->getVal() << " is NOT in interval" << endl;

  s->setVal(170);
  paramOfInterest.Print("v");
  if( lrint->IsInInterval(paramOfInterest) ) 
    cout << "s = " << s->getVal() << " is in interval" << endl;
  else
    cout << "s = " << s->getVal() << " is NOT in interval" << endl;
  //  cout << "ll = " << ((LikelihoodInterval*) lrint)->LowerLimit(*s) << endl;
  //  cout << "ul = " << ((LikelihoodInterval*) lrint)->UpperLimit(*s) << endl;
  // now use a tool


}
