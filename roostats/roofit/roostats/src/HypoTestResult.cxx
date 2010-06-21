// @(#)root/roostats:$Id$
// Author: Kyle Cranmer, Lorenzo Moneta, Gregory Schott, Wouter Verkerke, Sven Kreiss
/*************************************************************************
 * Copyright (C) 1995-2008, Rene Brun and Fons Rademakers.               *
 * All rights reserved.                                                  *
 *                                                                       *
 * For the licensing terms see $ROOTSYS/LICENSE.                         *
 * For the list of contributors see $ROOTSYS/README/CREDITS.             *
 *************************************************************************/

/*****************************************************************************
 * Project: RooStats
 * Package: RooFit/RooStats  
 * @(#)root/roofit/roostats:$Id$
 * Authors:                     
 *   Kyle Cranmer, Lorenzo Moneta, Gregory Schott, Wouter Verkerke, Sven Kreiss
 *
 *****************************************************************************/



//_________________________________________________
/*
BEGIN_HTML
<p>
HypoTestResult is an base class for a results from hypothesis tests.  
Any tool inheriting from HypoTestCalculator can return a HypoTestResult.
As such, it stores a p-value for the null-hypothesis (eg. background-only) 
and an alternate hypothesis (eg. signal+background).  
The p-values can also be transformed into confidence levels (CLb, CLsplusb) in a trivial way.
The ratio of the CLsplusb to CLb is often called CLs, and is considered useful, though it is 
not a probability.
Finally, the p-value of the null can be transformed into a number of equivalent Gaussian sigma using the 
Significance method.
END_HTML
*/
//

#include "RooStats/HypoTestResult.h"
#include "RooAbsReal.h"

#ifndef RooStats_RooStatsUtils
#include "RooStats/RooStatsUtils.h"
#endif

#include <limits>
#define NaN numeric_limits<float>::quiet_NaN()
#define IsNaN(a) isnan(a)

ClassImp(RooStats::HypoTestResult) ;

using namespace RooStats;

//____________________________________________________________________
HypoTestResult::HypoTestResult(const char* name) : 
   TNamed(name,name),
   fNullPValue(NaN), fAlternatePValue(NaN),
   fTestStatisticData(NaN),
   fNullDistr(NULL), fAltDistr(NULL),
   fPValueIsRightTail(kTRUE)
{
   // Default constructor
}


//____________________________________________________________________
HypoTestResult::HypoTestResult(const char* name, Double_t nullp, Double_t altp) :
   TNamed(name,name),
   fNullPValue(nullp), fAlternatePValue(altp),
   fTestStatisticData(NaN),
   fNullDistr(NULL), fAltDistr(NULL),
   fPValueIsRightTail(kTRUE)
{
   // Alternate constructor
}


//____________________________________________________________________
HypoTestResult::~HypoTestResult()
{
   // Destructor

}


//____________________________________________________________________
void HypoTestResult::SetAltDistribution(const SamplingDistribution *alt) {
   fAltDistr = alt;
   UpdatePValue(fAltDistr, &fAlternatePValue, !fPValueIsRightTail);
}
//____________________________________________________________________
void HypoTestResult::SetNullDistribution(const SamplingDistribution *null) {
   fNullDistr = null;
   UpdatePValue(fNullDistr, &fNullPValue, fPValueIsRightTail);
}
//____________________________________________________________________
void HypoTestResult::SetTestStatisticData(const Double_t tsd) {
   fTestStatisticData = tsd;

   UpdatePValue(fNullDistr, &fNullPValue, fPValueIsRightTail);
   UpdatePValue(fAltDistr, &fAlternatePValue, !fPValueIsRightTail);
}
//____________________________________________________________________
void HypoTestResult::SetPValueIsRightTail(Bool_t pr) {
   fPValueIsRightTail = pr;

   UpdatePValue(fNullDistr, &fNullPValue, fPValueIsRightTail);
   UpdatePValue(fAltDistr, &fAlternatePValue, !fPValueIsRightTail);
}


Bool_t HypoTestResult::HasTestStatisticData(void) const {
   return !IsNaN(fTestStatisticData);
}


// private
//____________________________________________________________________
void HypoTestResult::UpdatePValue(const SamplingDistribution* distr, Double_t *pvalue, Bool_t pIsRightTail) {
   // updates the pvalue if sufficient data is available

   if(IsNaN(fTestStatisticData)) return;

   if(distr) {
      if(pIsRightTail)
         *pvalue = distr->Integral(-RooNumber::infinity(), fTestStatisticData);
      else
         *pvalue = distr->Integral(fTestStatisticData, RooNumber::infinity());
   }
}

