// @(#)root/roostats:$Id: ConfidenceBelt.cxx 26317 2009-01-13 15:31:05Z cranmer $
// Author: Kyle Cranmer, Lorenzo Moneta, Gregory Schott, Wouter Verkerke
/*************************************************************************
 * Copyright (C) 1995-2008, Rene Brun and Fons Rademakers.               *
 * All rights reserved.                                                  *
 *                                                                       *
 * For the licensing terms see $ROOTSYS/LICENSE.                         *
 * For the list of contributors see $ROOTSYS/README/CREDITS.             *
 *************************************************************************/

#ifndef RooStats_ConfidenceBelt
#define RooStats_ConfidenceBelt

#ifndef ROO_ARG_SET
#include "RooArgSet.h"
#endif
#ifndef ROO_TREE_DATA
#include "RooTreeData.h"
#endif
#ifndef RooStats_ConfInterval
#include "RooStats/ConfInterval.h"
#endif

#include <vector>
#include <map>

using namespace std;

namespace RooStats {
 class ConfidenceBelt : public TNamed {
    typedef pair<Double_t, Double_t> RangeType; // [min,max] of test statistic
    typedef map<Int_t, RangeType> BeltType; // int is index into RooTreeData
    typedef pair<Double_t, BeltType> BeltWithCLType; // Double is confidence level
    typedef vector<BeltWithCLType> MultipleBeltType; // Double is confidence level

  private:
    //    RooArgSet* fParameters; // parameter of interest
    MultipleBeltType fBelts; // confidence level
    RooTreeData* fParameterPointsInInterval; // either a histogram (RooDataHist) or a tree (RooDataSet)

  public:
    // constructors,destructors
    ConfidenceBelt();
    ConfidenceBelt(const char* name);
    ConfidenceBelt(const char* name, const char* title);
    ConfidenceBelt(const char* name, RooTreeData&);
    ConfidenceBelt(const char* name, const char* title, RooTreeData&);
    virtual ~ConfidenceBelt();
        
    RangeType* AcceptanceRegion(RooArgSet&, Double_t cl=-1.);
    Double_t AcceptanceRegionMin(RooArgSet&, Double_t cl=-1.);
    Double_t AcceptanceRegionMax(RooArgSet&, Double_t cl=-1.);
    vector<Double_t> ConfidenceLevels() const ;
 
    // Method to return lower limit on a given parameter 
    //  Double_t LowerLimit(RooRealVar& param) ; // could provide, but misleading?
    //      Double_t UpperLimit(RooRealVar& param) ; // could provide, but misleading?
    
    // do we want it to return list of parameters
    virtual RooArgSet* GetParameters() const;

    // check if parameters are correct. (dummy implementation to start)
    Bool_t CheckParameters(RooArgSet&) const ;


    
  protected:
    ClassDef(ConfidenceBelt,1)  // A confidence belt for the Neyman Construction
      
  };
}

#endif
