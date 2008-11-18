// @(#)root/roostats:$Id: ConfInterval.h 24970 2008-10-10  $
// Author: Kyle Cranmer, Lorenzo Moneta, Gregory Schott, Wouter Verkerke
/*************************************************************************
 * Copyright (C) 1995-2008, Rene Brun and Fons Rademakers.               *
 * All rights reserved.                                                  *
 *                                                                       *
 * For the licensing terms see $ROOTSYS/LICENSE.                         *
 * For the list of contributors see $ROOTSYS/README/CREDITS.             *
 *************************************************************************/

#ifndef ROOSTATS_ConfInterval
#define ROOSTATS_ConfInterval

#include "TNamed.h"
#include "RooArgSet.h"

//_________________________________________________________________
//
// BEGIN_HTML
// ConfInterval is an interface class for a generic interval in the RooStats framework.
// Any tool inheriting from IntervalCalculator can return a ConfInterval.
// There are many types of intervals, they may be a simple range [a,b] in 1 dimension,
// or they may be disconnected regions in multiple dimensions.
// So the common interface is simply to ask the interval if a given point "IsInInterval".
// The Interval also knows what confidence level it was constructed at and the space of 
// parameters for which it was constructed.
// Note, one could use the same class for a Bayesian "credible interval".
// END_HTML
//
//


namespace RooStats {

  class ConfInterval : public TNamed {
  public:
    ConfInterval();
    ConfInterval(const char* name);
    ConfInterval(const char* name, const char* title);
    virtual ~ConfInterval();
    
    //pure virtual?  where does =0 go with const?
    virtual Bool_t IsInInterval(RooArgSet&) = 0; 
    
    // used to set confidence level.  Keep pure virtual
    virtual void SetConfidenceLevel(Double_t cl) = 0;
    // return confidence level
    virtual Double_t ConfidenceLevel() const = 0;
    // 
    // if so does this implement it?
    // private fSize;
    

    // do we want it to return list of parameters
    RooArgSet& GetParameters();

    // check if parameters are correct. (dummy implementation to start)
    Bool_t CheckParameters(RooArgSet&) {return true;}


  protected:
    ClassDef(ConfInterval,1)


   };
}

// Without this macro the THtml doc for TMath can not be generated
//#if !defined(R__ALPHA) && !defined(R__SOLARIS) && !defined(R__ACC) && !defined(R__FBSD)
//NamespaceImp(RooStats)
//#endif

ClassImp(RooStats::ConfInterval) ;

#endif
