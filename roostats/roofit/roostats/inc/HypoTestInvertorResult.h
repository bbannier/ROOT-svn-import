#ifndef ROOSTATS_HypoTestInvertorResult
#define ROOSTATS_HypoTestInvertorResult

#include "TNamed.h"
#include "RooRealVar.h"
#include "RooStats/SimpleInterval.h"


namespace RooStats {

  class HypoTestInvertorResult : public SimpleInterval /*, public TNamed*/ {

  public:

    // constructor
    HypoTestInvertorResult( const char* name, 
			    const char* title,
			    RooRealVar* scannedVariable,
			    double cl ) ;

    // destructor
    virtual ~HypoTestInvertorResult() ; // TO DO DELETE ALL yOBJECTS

    // function to return the yValue

    //SimpleInterval* GetInterval() const ; 

    double GetXValue( int index ) ;

    double GetYValue( int index ) ;

    int Size() { return xValues.size(); };

    std::vector<double> xValues;
    TList yObjects;

    Double_t LowerLimit() { CalculateLimits(); return fLowerLimit; }
    Double_t UpperLimit() { CalculateLimits(); return fUpperLimit; }

  private:

    void CalculateLimits() ;

/*     Double_t fLowerLimit;         // lower limit on the constrained variable */
/*     Double_t fUpperLimit;         // upper limit on the constrained variable */
/*     RooAbsArg* fScannedVariable;  // pointer to the constrained variable */
    
  protected:

    ClassDef(HypoTestInvertorResult,1)  // HypoTestInvertorResult class

  };
}

#endif
