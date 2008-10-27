// @(#)root/roostats:$Id: HypoTestResult.h 24970 2008-10-10  $
// Author: Kyle Cranmer, Lorenzo Moneta, Gregory Schott, Wouter Verkerke
/*************************************************************************
 * Copyright (C) 1995-2008, Rene Brun and Fons Rademakers.               *
 * All rights reserved.                                                  *
 *                                                                       *
 * For the licensing terms see $ROOTSYS/LICENSE.                         *
 * For the list of contributors see $ROOTSYS/README/CREDITS.             *
 *************************************************************************/

#ifndef RooStats_HypoTestResult
#define RooStats_HypoTestResult

#include "TNamed.h"

namespace RooStats {
  class HypoTestResult { //: TNamed {
  public:
    HypoTestResult();
    virtual ~HypoTestResult();
    
    //pure virtual? 
    virtual Double_t NullPValue();
    //pure virtual? 
    virtual Double_t AlternatePValue();
    
    // familiar name for NullPValue()
    virtual Double_t CLb();
    // familiar name for AlternatePValue()
    virtual Double_t CLsplusb();
    // CLs is simply CLs+b/CLb (not a method, but a quantity)
    virtual Double_t CLs();
    
    // familiar name for the Null p-value in terms of 1-sided Gaussian significance
    virtual Double_t Significance();
    
  private:
    Double_t fNullPValue;
    Double_t fAlternatePValue;
    
  protected:
    ClassDef(HypoTestResult,1)  
      
   };
}
#endif
