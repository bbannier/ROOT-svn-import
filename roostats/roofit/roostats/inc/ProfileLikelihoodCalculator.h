// @(#)root/roostats:$Id: ProfileLikelihoodCalculator.h 24970 2008-10-10  $
// Author: Kyle Cranmer, Lorenzo Moneta, Gregory Schott, Wouter Verkerke
/*************************************************************************
 * Copyright (C) 1995-2008, Rene Brun and Fons Rademakers.               *
 * All rights reserved.                                                  *
 *                                                                       *
 * For the licensing terms see $ROOTSYS/LICENSE.                         *
 * For the list of contributors see $ROOTSYS/README/CREDITS.             *
 *************************************************************************/

#ifndef ROOSTATS_ProfileLikelihoodCalculator
#define ROOSTATS_ProfileLikelihoodCalculator

#include "RooStats/CombinedCalculator.h"
#include "RooAbsPdf.h"
#include "RooArgSet.h"

namespace RooStats {
 class ProfileLikelihoodCalculator : public CombinedCalculator {
  public:
    ProfileLikelihoodCalculator();
    virtual ~ProfileLikelihoodCalculator();
    
    // main interface, implemented
    virtual ConfInterval* GetInterval() const ; 
    // main interface, implemented
    virtual HypoTestResult* GetHypoTest() const;   
    

  protected:
    ClassDef(ProfileLikelihoodCalculator,1) // A concrete implementation of CombinedCalculator that uses the ProfileLikelihood ratio.
  };
}
#endif
