// @(#)root/roostats:$Id: ConfInterval.h 24970 2008-10-10  cranmer $
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



// This is an interface class for confidence/credible intervals/regions.  Source file just for constructors, this class cannot be instantiated.

#ifndef RooStats_ConfInterval
#include "RooStats/ConfInterval.h"
#endif
#include "RooAbsReal.h"

// Without this macro the THtml doc for TMath can not be generated
#if !defined(R__ALPHA) && !defined(R__SOLARIS) && !defined(R__ACC) && !defined(R__FBSD)
NamespaceImp(RooStats)
#endif

ClassImp(RooStats::ConfInterval) ;

using namespace RooStats;

//____________________________________________________________________
ConfInterval::ConfInterval()
{
  // Default constructor
}

//____________________________________________________________________
ConfInterval::ConfInterval(const char* name) :
  TNamed(name,name)
{
  // Alternate constructor
}

//____________________________________________________________________
ConfInterval::ConfInterval(const char* name, const char* title):
  TNamed(name,title)
{
  // Alternate constructor
}

//____________________________________________________________________
ConfInterval::~ConfInterval()
{
  // Destructor
}
