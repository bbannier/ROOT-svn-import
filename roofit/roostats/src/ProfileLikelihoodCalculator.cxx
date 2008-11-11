// @(#)root/roostats:$Id: ProfileLikelihoodCalculator.cxx  $
// Author: Kyle Cranmer   28/07/2008

/*************************************************************************
 * Copyright (C) 1995-2008, Rene Brun and Fons Rademakers.               *
 * All rights reserved.                                                  *
 *                                                                       *
 * For the licensing terms see $ROOTSYS/LICENSE.                         *
 * For the list of contributors see $ROOTSYS/README/CREDITS.             *
 *************************************************************************/

/////////////////////////////////////////
// ProfileLikelihoodCalculator
//

#ifndef RooStats_ProfileLikelihoodCalculator
#include "RooStats/ProfileLikelihoodCalculator.h"
#endif

#ifndef RooStats_RooStatsUtils
#include "RooStats/RooStatsUtils.h"
#endif

#include "RooStats/LikelihoodInterval.h"

#include "RooFitResult.h"
#include "RooRealVar.h"
#include "RooProfileLL.h"
#include "RooNLLVar.h"
#include "RooDataSet.h"
#include "RooGlobalFunc.h"
#include "RooCmdArg.h"


// Without this macro the THtml doc  can not be generated
#if !defined(R__ALPHA) && !defined(R__SOLARIS) && !defined(R__ACC) && !defined(R__FBSD)
NamespaceImp(RooStats)
#endif


//ClassImp(RooStats::ProfileLikelihoodCalculator) ;


// A combined calculator tool that uses the profile likelihood ratio to eliminate nuisance parameters.
// Essentially this is the method of MINUIT/MINOS.


using namespace RooFit;
using namespace RooStats;


//_______________________________________________________
ProfileLikelihoodCalculator::ProfileLikelihoodCalculator() : 
CombinedCalculator() {
  // default constructor

}

//_______________________________________________________
ProfileLikelihoodCalculator::~ProfileLikelihoodCalculator(){
  // destructor
}


//_______________________________________________________
ConfInterval* ProfileLikelihoodCalculator::GetInterval() const {

  RooAbsPdf* pdf   = fWS->pdf(fPdfName);
  RooAbsData* data = fWS->data(fDataName);
  if (!data || !pdf || !fPOI) return 0;

  RooNLLVar* nll = new RooNLLVar("nll","",*pdf,*data);
  RooProfileLL* profile = new RooProfileLL("pll","",*nll, *fPOI);
  profile->addOwnedComponents(*nll) ;  // to avoid memory leak

  RooMsgService::instance().setGlobalKillBelow(RooMsgService::FATAL) ;
  profile->getVal();
  RooMsgService::instance().setGlobalKillBelow(RooMsgService::DEBUG) ;

  LikelihoodInterval* interval 
    = new LikelihoodInterval("LikelihoodInterval", profile);

  return interval;
}

//_______________________________________________________
HypoTestResult* ProfileLikelihoodCalculator::GetHypoTest() const {

  RooAbsPdf* pdf   = fWS->pdf(fPdfName);
  RooAbsData* data = fWS->data(fDataName);
  if (!data || !pdf) return 0;

  // calculate MLE
  RooFitResult* fit = pdf->fitTo(*data,Extended(kFALSE),Strategy(0),Hesse(kFALSE),Save(kTRUE),PrintLevel(-1));
  

  fit->Print();
  Double_t NLLatMLE= fit->minNll();


  // set POI to null values, set constant, calculate conditional MLE
  TIter it = fNullParams->createIterator();
  RooRealVar *myarg; 
  RooRealVar *mytarget; 
  while ((myarg = (RooRealVar *)it.Next())) { 
    if(!myarg) continue;

    mytarget = fWS->var(myarg->GetName());
    if(!mytarget) continue;
    mytarget->setVal( myarg->getVal() );
    mytarget->setConstant(kTRUE);
    cout << "setting null parameter:" << endl;
    mytarget->Print();
  }
  
  RooFitResult* fit2 = pdf->fitTo(*data,Extended(kFALSE),Hesse(kFALSE),Strategy(0), Minos(kFALSE), Save(kTRUE),PrintLevel(-1));

  Double_t NLLatCondMLE= fit2->minNll();
  fit2->Print();

  // Use Wilks' theorem to translate -2 log lambda into a signifcance/p-value
  HypoTestResult* htr = 
    new HypoTestResult("ProfileLRHypoTestResult",
		       SignificanceToPValue(sqrt( 2*(NLLatCondMLE-NLLatMLE))), 0 );
  
  return htr;

}

