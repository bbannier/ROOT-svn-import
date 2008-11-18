// @(#)root/roostats:$Id: SimpleInterval.h 24970 2008-10-10  cranmer $
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



//_________________________________________________________
//
// BEGIN_HTML
// SimpleInterval is a concrete implementation of the ConfInterval interface.  
// It implements simple 1-dimensional intervals in a range [a,b].
// In addition, you can ask it for the upper- or lower-bound.
// END_HTML
//
//

#ifndef RooStats_SimpleInterval
#include "RooStats/SimpleInterval.h"
#endif
#include "RooAbsReal.h"

// Without this macro the THtml doc for TMath can not be generated
#if !defined(R__ALPHA) && !defined(R__SOLARIS) && !defined(R__ACC) && !defined(R__FBSD)
NamespaceImp(RooStats)
#endif

ClassImp(RooStats::SimpleInterval) ;

using namespace RooStats;

//____________________________________________________________________
SimpleInterval::SimpleInterval() : fLowerLimit(0), fUpperLimit(0)
{
  // Default constructor
}

//____________________________________________________________________
SimpleInterval::SimpleInterval(const char* name) :
  ConfInterval(name,name), fLowerLimit(0), fUpperLimit(0)
{
  // Alternate constructor
}

//____________________________________________________________________
SimpleInterval::SimpleInterval(const char* name, const char* title) :
  ConfInterval(name,title), fLowerLimit(0), fUpperLimit(0)
{
  // Alternate constructor
}

//____________________________________________________________________
SimpleInterval::SimpleInterval(const char* name, Double_t lower, Double_t upper) :
  ConfInterval(name,name), fLowerLimit(lower), fUpperLimit(upper)
{
  // Alternate constructor
}

//____________________________________________________________________
SimpleInterval::SimpleInterval(const char* name, const char* title, Double_t lower, Double_t upper):
  ConfInterval(name,title), fLowerLimit(lower), fUpperLimit(upper)
{
  // Alternate constructor
}


//____________________________________________________________________
SimpleInterval::~SimpleInterval()
{
  // Destructor

}


//____________________________________________________________________
Bool_t SimpleInterval::IsInInterval(RooArgSet &parameterPoint) 
{  

  // Method to determine if a parameter point is in the interval
  if( !this->CheckParameters(parameterPoint) )
    return false; 

  if(parameterPoint.getSize() != 1 )
    return false;

  RooAbsReal* point = dynamic_cast<RooAbsReal*> (parameterPoint.first());
  if (point == 0)
    return false;

  if ( point->getVal() > fUpperLimit || point->getVal() < fUpperLimit)
    return false;


  return true;
  
}
