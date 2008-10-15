// @(#)root/roostats:$Id: ConfInterval.h 24970 2008-10-10  $
// Author: Kyle Cranmer, Lorenzo Monetta, Gregory Schott, Wouter Verkerke
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

namespace RooStats {

  class ConfInterval {//: TNamed {
  public:
    ConfInterval();
    virtual ~ConfInterval() {}
    
    //pure virtual?  where does =0 go with const?
    virtual Bool_t IsInInterval(RooArgSet&) = 0; 
    
    // do we want it to cache confidence level?
    // double ConfLevel() const;
    // 
    // if so does this implement it?
    // private fSize;
    

    // do we want it to return list of parameters
    RooArgSet& GetParameters();

    // check if parameters are correct. (dummy implementation to start)
    Bool_t CheckParameters(RooArgSet&) {return true;}


  protected:
    //    ClassDef(ConfInterval,1)

   };
}
#endif
