// @(#)root/roostats:$Id: SimpleInterval.h 30478 2009-09-25 19:42:07Z schott $
// Author: Kyle Cranmer, Lorenzo Moneta, Gregory Schott, Wouter Verkerke
/*************************************************************************
 * Copyright (C) 1995-2008, Rene Brun and Fons Rademakers.               *
 * All rights reserved.                                                  *
 *                                                                       *
 * For the licensing terms see $ROOTSYS/LICENSE.                         *
 * For the list of contributors see $ROOTSYS/README/CREDITS.             *
 *************************************************************************/

#ifndef ROOSTATS_HypoTestInverter
#define ROOSTATS_HypoTestInverter



#ifndef ROOSTATS_IntervalCalculator
#include "RooStats/IntervalCalculator.h"
#endif

#ifndef ROOSTATS_HypoTestCalculator
#include "RooStats/HypoTestCalculator.h"
#endif

#ifndef  ROOSTATS_HypoTestInverterResult
#include "RooStats/HypoTestInverterResult.h"
#endif

class RooRealVar;


namespace RooStats {

  class HypoTestInverter : public IntervalCalculator, public TNamed {

  private:
    // default constructor (make private, used only for I/O)
    HypoTestInverter( const char* name,
		      const char* title ) ;

  public:

    // constructor
    HypoTestInverter( const char* name, 
		      const char* title, 
		      HypoTestCalculator* myhc0,
		      RooRealVar* scannedVariable  ) ;

    virtual HypoTestInverterResult* GetInterval() const { return fResults; } ;

    bool RunAutoScan( double xMin, double xMax, double epsilon );

    bool RunFixedScan( int nBins, double xMin, double xMax );

    bool RunOnePoint( double thisX );

    virtual void  SetData(RooAbsData &) { } // not needed

    virtual void SetModel(const ModelConfig &) { } // not needed 

    // set the size of the test (rate of Type I error) ( Eg. 0.05 for a 95% Confidence Interval)
    virtual void SetTestSize(Double_t size) {fSize = size; fResults->SetConfidenceLevel(1-fSize); }
    // set the confidence level for the interval (eg. 0.95 for a 95% Confidence Interval)
    virtual void SetConfidenceLevel(Double_t cl) {fSize = 1.-cl; fResults->SetConfidenceLevel(1-fSize); }
    // Get the size of the test (eg. rate of Type I error)
    virtual Double_t Size() const {return fSize;}
    // Get the Confidence level for the test
    virtual Double_t ConfidenceLevel()  const {return 1.-fSize;}
 
    // destructor
    virtual ~HypoTestInverter() ;

  private:

    HypoTestCalculator* fCalculator0;   // pointer to the calculator passed in the constructor
    RooRealVar* fScannedVariable;     // pointer to the constrained variable
    HypoTestInverterResult* fResults;
    
    double fSize;

  protected:

    ClassDef(HypoTestInverter,1)  // HypoTestInverter class

  };
}

#endif
