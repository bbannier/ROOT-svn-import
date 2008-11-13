// @(#)root/roostats:$Id: CombinedCalculator.h 24970 2008-10-10  $
// Author: Kyle Cranmer, Lorenzo Moneta, Gregory Schott, Wouter Verkerke
/*************************************************************************
 * Copyright (C) 1995-2008, Rene Brun and Fons Rademakers.               *
 * All rights reserved.                                                  *
 *                                                                       *
 * For the licensing terms see $ROOTSYS/LICENSE.                         *
 * For the list of contributors see $ROOTSYS/README/CREDITS.             *
 *************************************************************************/

#ifndef ROOSTATS_CombinedCalculator
#define ROOSTATS_CombinedCalculator

#include "TNamed.h"
#include "RooStats/IntervalCalculator.h"
#include "RooStats/HypoTestCalculator.h"
#include "RooAbsPdf.h"
#include "RooArgSet.h"
#include "RooWorkspace.h"

// An base class for calculators that can fulfil both the 
// IntervalCalculator and HypoTestCalculator interfaces.

namespace RooStats {
 class CombinedCalculator : public IntervalCalculator, public HypoTestCalculator {
  public:
    CombinedCalculator(){
       fWS = 0;
       fNullParams = 0;
       fAlternateParams = 0;
       fPOI = 0;
       fNuisParams = 0;
    }

    virtual ~CombinedCalculator() {
      if (fWS) delete fWS;
      if (fNullParams) delete fNullParams;
      if (fAlternateParams) delete fAlternateParams;
      //      if (fPOI) delete fPOI;
      if (fNuisParams) delete fNuisParams;
    }

    
    // main interface, keep pure virtual
    virtual ConfInterval* GetInterval() const = 0; 
    // main interface, pure virtual
    virtual HypoTestResult* GetHypoTest() const = 0;   

    // from IntervalCalculator
    // Implement easy stuff:
    virtual void SetSize(Double_t size) {fSize = size;}
    virtual void SetConfidenceLevel(Double_t cl) {fSize = 1.-cl;}
    virtual Double_t Size() const {return fSize;}
    virtual Double_t ConfidenceLevel()  const {return 1.-fSize;}
    

    virtual void SetWorkspace(RooWorkspace* ws) {fWS = ws;}
    virtual void SetPdf(const char* name) {fPdfName = name;}
    virtual void SetData(const char* name){fDataName = name;}
    virtual void SetParameters(RooArgSet* set) {fPOI = set;}
    virtual void SetNuisanceParameters(RooArgSet* set) {fNuisParams = set;}
    

    // from HypoTestCalculator
    // set the PDF for the null hypothesis
    virtual void SetNullPdf(const char* name) {SetPdf(name);}
    // set the PDF for the alternate hypothesis
    virtual void SetAlternatePdf(const char* name) {SetPdf(name);}
    // set a common PDF for both the null and alternate hypotheses
    virtual void SetCommonPdf(const char* name) {SetPdf(name);}
    // set parameter values for the null if using a common PDF
    virtual void SetNullParameters(RooArgSet* set) {fNullParams = set;}
    // set parameter values for the alternate if using a common PDF
    virtual void SetAlternateParameters(RooArgSet* set) {fAlternateParams = set;}
    


  protected:

    Double_t fSize; // size of the test (eg. specified rate of Type I error)
    RooWorkspace* fWS;
    const char* fPdfName; // should be common
    const char* fDataName; // should be common
    RooArgSet* fNullParams;
    RooArgSet* fAlternateParams;
    RooArgSet* fPOI;
    RooArgSet* fNuisParams;

    //    ClassDef(CombinedCalculator,1)        
  };
}
#endif
