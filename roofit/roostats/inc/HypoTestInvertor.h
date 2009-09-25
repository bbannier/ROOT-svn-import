#ifndef ROOSTATS_HypoTestInvertor
#define ROOSTATS_HypoTestInvertor

#include "TGraph.h"
#include "TNamed.h"
#include "RooRealVar.h"
#include "RooStats/IntervalCalculator.h"
#include "RooStats/HypoTestInvertorResult.h"



namespace RooStats {

  class HypoTestInvertor : public IntervalCalculator, public TNamed {

  public:

    // default constructor
    HypoTestInvertor( const char* name,
		      const char* title ) ;

    // constructor
    HypoTestInvertor( const char* name, 
		      const char* title, 
		      HypoTestCalculator* myhc0,
		      RooRealVar* scannedVariable  ) ;

    virtual HypoTestInvertorResult* GetInterval() const { return fResults; } ;

    bool RunAutoScan( double xMin, double xMax, double epsilon );

    bool RunFixedScan( int nBins, double xMin, double xMax );

    bool RunOnePoint( double thisX );

    virtual void  SetData(RooAbsData &) { } // not needed

    // set the size of the test (rate of Type I error) ( Eg. 0.05 for a 95% Confidence Interval)
    virtual void SetTestSize(Double_t size) {fSize = size; fResults->SetConfidenceLevel(1-fSize); }
    // set the confidence level for the interval (eg. 0.95 for a 95% Confidence Interval)
    virtual void SetConfidenceLevel(Double_t cl) {fSize = 1.-cl; fResults->SetConfidenceLevel(1-fSize); }
    // Get the size of the test (eg. rate of Type I error)
    virtual Double_t Size() const {return fSize;}
    // Get the Confidence level for the test
    virtual Double_t ConfidenceLevel()  const {return 1.-fSize;}
 
    // destructor
    virtual ~HypoTestInvertor() ;

  private:

    HypoTestCalculator* fCalculator0;   // pointer to the calculator passed in the constructor
    RooRealVar* fScannedVariable;     // pointer to the constrained variable
    HypoTestInvertorResult* fResults;
    
    double fSize;

  protected:

    ClassDef(HypoTestInvertor,1)  // HypoTestInvertor class

  };
}

#endif
