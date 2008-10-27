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
HypoTestResult::HypoTestResult(const char* name) :
  TNamed(name,name)
{
  // Alternate constructor
}

//____________________________________________________________________
HypoTestResult::HypoTestResult(const char* name, const char* title):
  TNamed(name,title)
{
  // Alternate constructor
}


//____________________________________________________________________
HypoTestResult::~HypoTestResult()
{
  // Destructor

}

//____________________________________________________________________
Double_t HypoTestResult::NullPValue() {
  return fNullPValue;
}

//____________________________________________________________________
Double_t HypoTestResult::AlternatePValue()  {
  return fAlternatePValue;
}

//____________________________________________________________________
Double_t HypoTestResult::Significance() 
{  
  return PValueToSignificance( fNullPValue) ;  
}

    
//____________________________________________________________________
Double_t HypoTestResult::CLb()  {
  return NullPValue(); 
}

//____________________________________________________________________
Double_t HypoTestResult::CLsplusb()   {
  return AlternatePValue(); 
}

//____________________________________________________________________
Double_t HypoTestResult::CLs()   {
  return CLsplusb() / CLb();
}
 
