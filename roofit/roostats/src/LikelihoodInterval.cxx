// @(#)root/roostats:$Id: LikelihoodInterval.h 24970 2008-10-10  cranmer $
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



// This class implements the ConfInterval interface for a simple 1-d interval of the form [a,b]

#ifndef RooStats_LikelihoodInterval
#include "RooStats/LikelihoodInterval.h"
#endif
#include "RooAbsReal.h"
#include "RooStats/RooStatsUtils.h"

// Without this macro the THtml doc for TMath can not be generated
#if !defined(R__ALPHA) && !defined(R__SOLARIS) && !defined(R__ACC) && !defined(R__FBSD)
NamespaceImp(RooStats)
#endif

ClassImp(RooStats::LikelihoodInterval) ;

using namespace RooStats;

//____________________________________________________________________
LikelihoodInterval::LikelihoodInterval() : fLikelihoodRatio(0)
{
  // Default constructor
}

//____________________________________________________________________
LikelihoodInterval::LikelihoodInterval(const char* name) :
  ConfInterval(name,name), fLikelihoodRatio(0)
{
  // Alternate constructor
}

//____________________________________________________________________
LikelihoodInterval::LikelihoodInterval(const char* name, const char* title) :
  ConfInterval(name,title), fLikelihoodRatio(0)
{
  // Alternate constructor
}

//____________________________________________________________________
LikelihoodInterval::LikelihoodInterval(const char* name, RooAbsReal* lr) :
  ConfInterval(name,name), fLikelihoodRatio(lr)
{
  // Alternate constructor
}

//____________________________________________________________________
LikelihoodInterval::LikelihoodInterval(const char* name, const char* title, RooAbsReal* lr) :
  ConfInterval(name,title), fLikelihoodRatio(lr)
{
  // Alternate constructor
}

//____________________________________________________________________
LikelihoodInterval::~LikelihoodInterval()
{
  // Destructor

}


//____________________________________________________________________
Bool_t LikelihoodInterval::IsInInterval(RooArgSet &parameterPoint) 
{  

  // Method to determine if a parameter point is in the interval
  if( !this->CheckParameters(parameterPoint) )
    return false; 


  // set parameters
  SetParameters(&parameterPoint, fLikelihoodRatio->getVariables() );
  // evaluate likelihood ratio, see if it's bigger than threshold

  
  std::cout << "lr = " << fLikelihoodRatio->getVal() << " " << parameterPoint.getSize() << std::endl;
  if ( TMath::Prob( 2* fLikelihoodRatio->getVal(), parameterPoint.getSize()) < 0.05 )
    return false;


  return true;
  
}
