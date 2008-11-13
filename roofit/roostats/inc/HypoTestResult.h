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
#include "RooStats/RooStatsUtils.h"

namespace RooStats {
  class HypoTestResult  : public TNamed {
  public:
    HypoTestResult();
    HypoTestResult(const char* name, Double_t nullp, Double_t altp);
    HypoTestResult(const char* name, const char* title, Double_t nullp, Double_t altp);
    virtual ~HypoTestResult();
    
    virtual Double_t NullPValue() const {return fNullPValue;}
    virtual Double_t AlternatePValue() const {return fAlternatePValue;}
    
    // familiar name for NullPValue()
    virtual Double_t CLb() const {return 1.-NullPValue();}
    // familiar name for AlternatePValue()
    virtual Double_t CLsplusb() const {return 1.-AlternatePValue();}
    // CLs is simply CLs+b/CLb (not a method, but a quantity)
    virtual Double_t CLs() const {return CLsplusb()/CLb();}
    
    // familiar name for the Null p-value in terms of 1-sided Gaussian significance
    virtual Double_t Significance() const {return RooStats::PValueToSignificance( fNullPValue ); }
    
  private:
    Double_t fNullPValue;
    Double_t fAlternatePValue;
    
  protected:
    ClassDef(HypoTestResult,1)  
      
   };
}
#endif
