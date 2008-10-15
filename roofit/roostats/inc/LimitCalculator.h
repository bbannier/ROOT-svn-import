// @(#)root/roostats:$Id: LimitCalculator.h 24970 2008-10-10  $
// Author: Kyle Cranmer, Lorenzo Moneta, Gregory Schott, Wouter Verkerke
/*************************************************************************
 * Copyright (C) 1995-2008, Rene Brun and Fons Rademakers.               *
 * All rights reserved.                                                  *
 *                                                                       *
 * For the licensing terms see $ROOTSYS/LICENSE.                         *
 * For the list of contributors see $ROOTSYS/README/CREDITS.             *
 *************************************************************************/

#ifndef ROOSTATS_LimitCalculator
#define ROOSTATS_LimitCalculator

#include "TNamed.h"
#include "RooStats/ConfInterval.h"
#include "RooAbsPdf.h"
#include "RooArgSet.h"

namespace RooStats {
  class LimitCalculator { //: TNamed {
  public:
    LimitCalculator();
    virtual ~LimitCalculator();
    
    //pure virtual?  
    virtual ConfInterval* GetInterval() const = 0; 

    // set the size of the test (rate of Type I error).  
    // Eg. 0.5 for a 95% Confidence Interval
    virtual void SetConfidenceLevel(Double_t) = 0;
    
    // do we want it to cache confidence level?
    // Double_t Size() const;
    // Double_t ConfidenceLevel() const; // different name
    // if so does this implement it?
    // private fSize;
    

    virtual void SetPdf(RooAbsPdf&) = 0;
    virtual void SetParameters(RooArgSet&) = 0;
    virtual void SetNuisanceParameters(RooArgSet&) = 0;
    

  protected:
    ClassDef(LimitCalculator,1)        
  };
}
#endif
