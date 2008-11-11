// @(#)root/roostats:$Id: SimpleInterval.h 24970 2008-10-10  $
// Author: Kyle Cranmer, Lorenzo Moneta, Gregory Schott, Wouter Verkerke
/*************************************************************************
 * Copyright (C) 1995-2008, Rene Brun and Fons Rademakers.               *
 * All rights reserved.                                                  *
 *                                                                       *
 * For the licensing terms see $ROOTSYS/LICENSE.                         *
 * For the list of contributors see $ROOTSYS/README/CREDITS.             *
 *************************************************************************/

#ifndef RooStats_SimpleInterval
#define RooStats_SimpleInterval

#ifndef ROO_ARG_SET
#include "RooArgSet.h"
#endif
#ifndef RooStats_ConfInterval
#include "RooStats/ConfInterval.h"
#endif

namespace RooStats {
 class SimpleInterval : public ConfInterval {
  public:
    // constructors,destructors
    SimpleInterval();
    SimpleInterval(const char* name);
    SimpleInterval(const char* name, const char* title);
    SimpleInterval(const char* name, Double_t, Double_t);
    SimpleInterval(const char* name, const char* title, Double_t, Double_t);
    virtual ~SimpleInterval();
        
    virtual Bool_t IsInInterval(RooArgSet&);
    virtual void SetConfidenceLevel(Double_t cl) {fConfidenceLevel = cl;}
    virtual Double_t ConfidenceLevel() const {return fConfidenceLevel;}
 
    // Method to return lower limit
    Double_t LowerLimit() {return fLowerLimit;}
    // Method to return upper limit
    Double_t UpperLimit() {return fUpperLimit;}
    
  private:
    Double_t fLowerLimit;
    Double_t fUpperLimit;
    Double_t fConfidenceLevel;
    
  protected:
    ClassDef(SimpleInterval,1)  
      
  };
}

#endif
