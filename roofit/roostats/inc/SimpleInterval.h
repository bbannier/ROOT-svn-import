// @(#)root/roostats:$Id: SimpleInterval.h 24970 2008-10-10  $
// Author: Kyle Cranmer, Lorenzo Monetta, Gregory Schott, Wouter Verkerke
/*************************************************************************
 * Copyright (C) 1995-2008, Rene Brun and Fons Rademakers.               *
 * All rights reserved.                                                  *
 *                                                                       *
 * For the licensing terms see $ROOTSYS/LICENSE.                         *
 * For the list of contributors see $ROOTSYS/README/CREDITS.             *
 *************************************************************************/

#ifndef ROOSTATS_SimpleInterval
#define ROOSTATS_SimpleInterval

#include "RooArgSet.h"
#include "RooStats/ConfInterval.h"

namespace RooStats {
 class SimpleInterval : public ConfInterval {
  public:
    SimpleInterval();
    virtual ~SimpleInterval();
    
    virtual Bool_t IsInInterval(RooArgSet&);
    
  private:
    Double_t fLowerLimit;
    Double_t fUpperLimit;
    
  protected:
    ClassDef(SimpleInterval,1)  
      
  };
}

#endif
