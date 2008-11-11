// @(#)root/roostats:$Id: LikelihoodInterval.h 24970 2008-10-10  $
// Author: Kyle Cranmer, Lorenzo Moneta, Gregory Schott, Wouter Verkerke
/*************************************************************************
 * Copyright (C) 1995-2008, Rene Brun and Fons Rademakers.               *
 * All rights reserved.                                                  *
 *                                                                       *
 * For the licensing terms see $ROOTSYS/LICENSE.                         *
 * For the list of contributors see $ROOTSYS/README/CREDITS.             *
 *************************************************************************/

#ifndef RooStats_LikelihoodInterval
#define RooStats_LikelihoodInterval

#ifndef ROO_ARG_SET
#include "RooArgSet.h"
#endif
#ifndef RooStats_ConfInterval
#include "RooStats/ConfInterval.h"
#endif

namespace RooStats {
 class LikelihoodInterval : public ConfInterval {
  public:
    // constructors,destructors
    LikelihoodInterval();
    LikelihoodInterval(const char* name);
    LikelihoodInterval(const char* name, const char* title);
    LikelihoodInterval(const char* name, RooAbsReal*);
    LikelihoodInterval(const char* name, const char* title, RooAbsReal*);
    virtual ~LikelihoodInterval();
        
    virtual Bool_t IsInInterval(RooArgSet&);
 
    // Method to return lower limit on a given parameter out of a set of parameters of interest
    //    Double_t LowerLimit(RooAbsReal* param, RooArgSet) const;
    //    Double_t LowerLimit(RooAbsReal* param, Int_t) const;
    // Method to return upper limit on a given parameter out of a set of parameters of interest
    //    Double_t UpperLimit(RooAbsReal* param) const;
    //    Double_t UpperLimit(RooAbsReal* param, Int_t) const;
    
  private:
    RooAbsReal* fLikelihoodRatio;
    
  protected:
    ClassDef(LikelihoodInterval,1)  
      
  };
}

#endif
