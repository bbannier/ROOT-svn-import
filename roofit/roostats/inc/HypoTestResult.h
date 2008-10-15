// @(#)root/roostats:$Id: HypoTestResult.h 24970 2008-10-10  $
// Author: Kyle Cranmer, Lorenzo Monetta, Gregory Schott, Wouter Verkerke
/*************************************************************************
 * Copyright (C) 1995-2008, Rene Brun and Fons Rademakers.               *
 * All rights reserved.                                                  *
 *                                                                       *
 * For the licensing terms see $ROOTSYS/LICENSE.                         *
 * For the list of contributors see $ROOTSYS/README/CREDITS.             *
 *************************************************************************/

#ifndef ROOSTATS_HypoTestResult
#define ROOSTATS_HypoTestResult

#include "TNamed.h"

namespace RooStats {
  class HypoTestResult { //: TNamed {
  public:
    HypoTestResult();
    virtual ~HypoTestResult();
    
    //pure virtual? 
    virtual Double_t NullPValue() {return fNullPValue;}
    //pure virtual? 
    virtual Double_t AlternatePValue()  {return fAlternatePValue;}
    
    // familiar name for NullPValue()
    virtual Double_t CLb()  {return NullPValue(); }
    // familiar name for AlternatePValue()
    virtual Double_t CLsplusb()   {return AlternatePValue(); }
    // CLs is simply CLs+b/CLb (not a method, but a quantity)
    virtual Double_t CLs()   {return CLsplusb() / CLb();}
    
    // familiar name for the Null p-value in terms of 1-sided Gaussian significance
    virtual Double_t Significance()  = 0;
    
  private:
    Double_t fNullPValue;
    Double_t fAlternatePValue;
    
  protected:
    ClassDef(HypoTestResult,1)  
      
   };
}
#endif
