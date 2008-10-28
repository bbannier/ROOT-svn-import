// @(#)root/roostats:$Id: HypoTestCalculator.h 24970 2008-10-10  $
// Author: Kyle Cranmer, Lorenzo Moneta, Gregory Schott, Wouter Verkerke
/*************************************************************************
 * Copyright (C) 1995-2008, Rene Brun and Fons Rademakers.               *
 * All rights reserved.                                                  *
 *                                                                       *
 * For the licensing terms see $ROOTSYS/LICENSE.                         *
 * For the list of contributors see $ROOTSYS/README/CREDITS.             *
 *************************************************************************/

#ifndef ROOSTATS_HypoTestCalculator
#define ROOSTATS_HypoTestCalculator

#include "TNamed.h"
#include "RooStats/HypoTestResult.h"
#include "RooAbsPdf.h"
#include "RooArgSet.h"

namespace RooStats {
  class HypoTestCalculator {
  public:

     virtual ~HypoTestCalculator() {}
    
    // main interface, pure virtual
    virtual HypoTestResult* GetHypoTest() const = 0;   

    // set the PDF for the null hypothesis
    virtual void SetNullPdf(const char* name) = 0;
    // set the PDF for the alternate hypothesis
    virtual void SetAlternatePdf(const char* name) = 0;
    // set a common PDF for both the null and alternate hypotheses
    virtual void SetCommonPdf(const char* name) = 0;
    // set parameter values for the null if using a common PDF
    virtual void SetNullParameters(RooArgSet*) = 0;
    // set parameter values for the alternate if using a common PDF
    virtual void SetAlternateParameters(RooArgSet*) = 0;
    

  protected:
    //    ClassDef(HypoTestCalculator,1)        
  };
}
#endif
