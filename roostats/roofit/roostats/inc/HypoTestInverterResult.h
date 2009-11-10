// @(#)root/roostats:$Id: SimpleInterval.h 30478 2009-09-25 19:42:07Z schott $
// Author: Kyle Cranmer, Lorenzo Moneta, Gregory Schott, Wouter Verkerke
/*************************************************************************
 * Copyright (C) 1995-2008, Rene Brun and Fons Rademakers.               *
 * All rights reserved.                                                  *
 *                                                                       *
 * For the licensing terms see $ROOTSYS/LICENSE.                         *
 * For the list of contributors see $ROOTSYS/README/CREDITS.             *
 *************************************************************************/

#ifndef ROOSTATS_HypoTestInverterResult
#define ROOSTATS_HypoTestInverterResult



#ifndef ROOSTATS_SimpleInterval
#include "RooStats/SimpleInterval.h"
#endif

class RooRealVar;

namespace RooStats {

  class HypoTestInverterResult : public SimpleInterval {

  public:

    // default constructor
    HypoTestInverterResult();

    // constructor
    HypoTestInverterResult( const char* name, 
			    const char* title,
			    RooRealVar* scannedVariable,
			    double cl ) ;

    // destructor
    virtual ~HypoTestInverterResult();

    // function to return the yValue

    double GetXValue( int index ) const ;

    double GetYValue( int index ) const ;
    double GetYError( int index ) const ;

    int Size() const { return fXValues.size(); };

    void UseCLs(bool on = true) { fUseCLs = on; }  

    Double_t LowerLimit() { CalculateLimits(); return fLowerLimit; }
    Double_t UpperLimit() { CalculateLimits(); return fUpperLimit; }

    Double_t UpperLimitError() { return 0; };

  private:

    void CalculateLimits() ;

  protected:

    bool fUseCLs; 

    std::vector<double> fXValues;

    TList fYObjects;

    friend class HypoTestInverter;

    ClassDef(HypoTestInverterResult,1)  // HypoTestInverterResult class

      };
}

#endif
