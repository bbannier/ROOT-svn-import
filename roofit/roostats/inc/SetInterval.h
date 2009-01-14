// @(#)root/roostats:$Id: SetInterval.cxx 26317 2009-01-13 15:31:05Z cranmer $
// Author: Kyle Cranmer, Lorenzo Moneta, Gregory Schott, Wouter Verkerke
/*************************************************************************
 * Copyright (C) 1995-2008, Rene Brun and Fons Rademakers.               *
 * All rights reserved.                                                  *
 *                                                                       *
 * For the licensing terms see $ROOTSYS/LICENSE.                         *
 * For the list of contributors see $ROOTSYS/README/CREDITS.             *
 *************************************************************************/

#ifndef RooStats_SetInterval
#define RooStats_SetInterval

#ifndef ROO_ARG_SET
#include "RooArgSet.h"
#endif
#ifndef ROO_TREE_DATA
#include "RooTreeData.h"
#endif
#ifndef RooStats_ConfInterval
#include "RooStats/ConfInterval.h"
#endif


namespace RooStats {
 class SetInterval : public ConfInterval {
  private:
    //    RooArgSet* fParameters; // parameter of interest
    Double_t fConfidenceLevel; // confidence level
    RooTreeData* fParameterPointsInInterval; // either a histogram (RooDataHist) or a tree (RooDataSet)

  public:
    // constructors,destructors
    SetInterval();
    SetInterval(const char* name);
    SetInterval(const char* name, const char* title);
    SetInterval(const char* name, RooTreeData&);
    SetInterval(const char* name, const char* title, RooTreeData&);
    virtual ~SetInterval();
        
    virtual Bool_t IsInInterval(RooArgSet&);
    virtual void SetConfidenceLevel(Double_t cl) {fConfidenceLevel = cl;}
    virtual Double_t ConfidenceLevel() const {return fConfidenceLevel;}
 
    // Method to return lower limit on a given parameter 
    //  Double_t LowerLimit(RooRealVar& param) ; // could provide, but misleading?
    //      Double_t UpperLimit(RooRealVar& param) ; // could provide, but misleading?
    
    // do we want it to return list of parameters
    virtual RooArgSet* GetParameters() const;

    // check if parameters are correct. (dummy implementation to start)
    Bool_t CheckParameters(RooArgSet&) const ;


    
  protected:
    ClassDef(SetInterval,1)  // Concrete implementation of ConfInterval for simple 1-D intervals in the form [a,b]
      
  };
}

#endif
