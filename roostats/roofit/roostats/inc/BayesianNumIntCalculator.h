#ifndef ROOSTATS_BayesianNumIntCalculator
#define ROOSTATS_BayesianNumIntCalculator

#include "TNamed.h"

#include "RooAbsArg.h"
#include "RooAbsData.h"
#include "RooArgSet.h"
#include "RooAbsPdf.h"
#include "RooAbsRealLValue.h"

#include "RooStats/IntervalCalculator.h"
#include "RooStats/SimpleInterval.h"


namespace RooStats {

  class BayesianNumIntCalculator : /*public IntervalCalculator,*/ public TNamed {

  public:

    // constructor
    BayesianNumIntCalculator( const char* name, 
			      const char* title );

    BayesianNumIntCalculator( const char* name, 
			      const char* title,
			      RooAbsPdf* pdf,
			      RooAbsData* data,
			      RooAbsRealLValue* POI,
			      RooAbsPdf* priorPOI,
			      RooArgSet* nuisanceParameters );

    // destructor
    ~BayesianNumIntCalculator() ;

    RooPlot* PlotPosterior() ; 

    SimpleInterval* GetInterval() ; 

  private:
    
    RooAbsPdf* fPdf;
    RooAbsData* fData;
    RooAbsRealLValue* fPOI;
    RooAbsPdf* fPriorPOI;
    RooArgSet* fNuisanceParameters;

    Double_t fLowerLimit;
    Double_t fUpperLimit;

  protected:

    ClassDef(BayesianNumIntCalculator,1)  // BayesianNumIntCalculator class

  };
}

#endif
