// @(#)root/roostats:$Id: HypoTestResult.cxx 24970 2008-10-10  cranmer $
// Author: Kyle Cranmer, Lorenzo Moneta, Gregory Schott, Wouter Verkerke
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
 *   Kyle Cranmer, Lorenzo Moneta, Gregory Schott, Wouter Verkerke
 *
 *****************************************************************************/



// This class implements the HypoTestResult

#include "RooStats/HypoTestResult.h"
#include "RooAbsReal.h"

#ifndef RooStats_RooStatsUtils
#include "RooStats/RooStatsUtils.h"
#endif

// Without this macro the THtml doc for TMath can not be generated
#if !defined(R__ALPHA) && !defined(R__SOLARIS) && !defined(R__ACC) && !defined(R__FBSD)
NamespaceImp(RooStats)
#endif

ClassImp(RooStats::HypoTestResult) ;

using namespace RooStats;

//____________________________________________________________________
HypoTestResult::HypoTestResult()
{
  // Default constructor
}


//____________________________________________________________________
HypoTestResult::HypoTestResult(const char* name, Double_t nullp, Double_t altp) :
  TNamed(name,name), fNullPValue(nullp), fAlternatePValue(altp)
{
  // Alternate constructor
}

//____________________________________________________________________
HypoTestResult::HypoTestResult(const char* name, const char* title, Double_t nullp, Double_t altp):
  TNamed(name,title), fNullPValue(nullp), fAlternatePValue(altp)
{
  // Alternate constructor
}


//____________________________________________________________________
HypoTestResult::~HypoTestResult()
{
  // Destructor

}
