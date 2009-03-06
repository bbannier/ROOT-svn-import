// @(#)root/roostats:$Id: ProfileLikelihoodCalculator.cxx 27519 2009-02-19 13:31:41Z pellicci $
// Author: Kyle Cranmer   28/07/2008

/*************************************************************************
 * Copyright (C) 1995-2008, Rene Brun and Fons Rademakers.               *
 * All rights reserved.                                                  *
 *                                                                       *
 * For the licensing terms see $ROOTSYS/LICENSE.                         *
 * For the list of contributors see $ROOTSYS/README/CREDITS.             *
 *************************************************************************/

//_________________________________________________
/*
BEGIN_HTML
<p>
BernsteinCorrection is a utility in RooStats to augment a nominal PDF with a polynomial 
correction term.  This is useful for incorporating systematic variations to the nominal PDF.  
The Bernstein basis polynomails are particularly appropriate because they are positive definite. 
</p>
<p>
This tool was inspired by the work of Glen Cowan together with Stephan Horner, Sasha Caron, Eilam Gross, and others.  
The initial implementation is independent work.  The major step forward in the approach was 
to provide a well defined algorithm that specifies the order of polynomial to be included 
in the correction.  This is an emperical algorithm, so in addition to the nominal model it 
needs either a real data set or a simulated one.  The algorithm basically consists of a 
hypothesis test of an nth-order correction (null) against a n+1-th order correction (alternate). 
The quantity q = -2 log LR is used to determine whether the n+1-th order correction is a major 
improvement to the n-th order correction.  The distribution of q is expected to be roughly 
\chi^2 with one degree of freedom if the n-th order correction is a good model for the data. 
 Thus, one only moves to the n+1-th order correction of q is relatively large.  The chance that 
one moves from the n-th to the n+1-th order correction when the n-th order correction 
(eg. a type 1 error) is sufficient is given by the Prob(\chi^2_1 > threshold).  The constructor 
of this class allows you to directly set this tolerance (in terms of probability that the n+1-th
 term is added unnecessarily).
</p>
<p>
To do:
Add another method to the utility that will make the sampling distribution for -2 log lambda 
for various m vs. m+1 order corrections using a nominal model and perhaps having two ways of 
generating the toys (either via a histogram or via an independent model that is supposed to
 reflect reality).  That will allow one to make plots like Glen has at the end of his DRAFT
 very easily. 
</p>
END_HTML
*/
//


#ifndef RooStats_BernsteinCorrection
#include "RooStats/BernsteinCorrection.h"
#endif 

#include "RooGlobalFunc.h"
#include "RooDataSet.h"
#include "RooRealVar.h"
#include "RooAbsPdf.h"
#include "RooFit.h"
#include "RooFitResult.h"
#include "TMath.h"
#include <string>
#include <vector>
#include <stdio.h>
#include <sstream>
#include <iostream>

#include "RooProdPdf.h"
#include "RooNLLVar.h"
#include "RooWorkspace.h"

#include "RooBernstein.h"


ClassImp(RooStats::BernsteinCorrection) ;

using namespace RooFit;
using namespace RooStats;

//____________________________________
BernsteinCorrection::BernsteinCorrection(Double_t tolerance):
  fMaxCorrection(100), fTolerance(tolerance){
}


//____________________________________
Int_t BernsteinCorrection::ImportCorrectedPdf(RooWorkspace* wks, 
					      const char* nominalName, 
					      const char* varName, 
					      const char* dataName){
  
  RooRealVar* x = wks->var(varName);
  RooAbsPdf* nominal = wks->pdf(nominalName);
  RooAbsData* data = wks->data(dataName);

  RooFitResult* nominalResult = nominal->fitTo(*data,Save());
  Double_t lastNll= nominalResult->minNll();
  cout << "first nll = " << lastNll << endl;
 
  RooArgList coeff;
  vector<RooRealVar*> coefficients;
  Double_t q = 1E6;
  Int_t degree = -1;
  bool keepGoing = true;
  while( keepGoing) {
    degree++;
    std::stringstream str;
    str<<"_"<<degree;

    RooRealVar* newCoef = new RooRealVar(("c"+str.str()).c_str(),
			"Bernstein basis poly coefficient", 
			1., 0., fMaxCorrection);
    coeff.add(*newCoef);
    coefficients.push_back(newCoef);

    RooBernstein* poly = new RooBernstein("poly", "Bernstein poly", *x, coeff);

    RooProdPdf* corrected = new RooProdPdf("corrected","",RooArgList(*nominal,*poly));

    RooMsgService::instance().setGlobalKillBelow(RooMsgService::FATAL) ;
    RooFitResult* result = corrected->fitTo(*data,Save(),Minos(kFALSE), Hesse(kFALSE),PrintLevel(-1));
    RooMsgService::instance().setGlobalKillBelow(RooMsgService::DEBUG) ;

    q = 2*(lastNll - result->minNll()); // -2 log lambda, goes like significance^2
    // check if we should keep going
    keepGoing = (degree < 1 || TMath::Prob(q,1) < fTolerance);
    if(!keepGoing){
      wks->import(*corrected);
      //wks->Print();
    } else { // memory management
      delete corrected;
      delete poly;
    }
    //    cout << "\n\ndegree = " << degree << " last nll = " << lastNll << " this nll = " 
    // << result->minNll() << " gof = " << q << endl;;
    lastNll = result->minNll();

    delete result;
  }

  return degree;
}
