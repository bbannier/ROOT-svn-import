// @(#)root/roostats:$Id: IntervalCalculator.h 24970 2008-10-10  $
// Author: Kyle Cranmer, Lorenzo Moneta, Gregory Schott, Wouter Verkerke
/*************************************************************************
 * Copyright (C) 1995-2008, Rene Brun and Fons Rademakers.               *
 * All rights reserved.                                                  *
 *                                                                       *
 * For the licensing terms see $ROOTSYS/LICENSE.                         *
 * For the list of contributors see $ROOTSYS/README/CREDITS.             *
 *************************************************************************/

#ifndef ROOSTATS_IntervalCalculator
#define ROOSTATS_IntervalCalculator

#include "TNamed.h"
#include "RooStats/ConfInterval.h"
#include "RooAbsPdf.h"
#include "RooArgSet.h"
#include "RooWorkspace.h"

namespace RooStats {
  class IntervalCalculator {//: public TNamed {
  public:
     //IntervalCalculator();
     virtual ~IntervalCalculator() {}
    
    // Main interface, pure virtual
    virtual ConfInterval* GetInterval() const = 0; 

    
    // keep these pure virtual
    virtual Double_t Size() const = 0;
    virtual Double_t ConfidenceLevel()  const = 0;  
    virtual void SetWorkspace(RooWorkspace* ws) = 0;
    virtual void SetPdf(const char* name) = 0;
    virtual void SetData(const char* name) = 0;
    virtual void SetParameters(RooArgSet&) = 0;
    virtual void SetNuisanceParameters(RooArgSet&) = 0;
    // set the size of the test (rate of Type I error).  
    // Eg. 0.05 for a 95% Confidence Interval
    virtual void SetSize(Double_t size) = 0;
    virtual void SetConfidenceLevel(Double_t cl) = 0;
    

  protected:
    //    ClassDef(IntervalCalculator,1)        
  };
}
#endif
