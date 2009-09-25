#ifndef ROOSTATS_HypoTestInvertorPlot
#define ROOSTATS_HypoTestInvertorPlot

#include "TGraph.h"
#include "TNamed.h"
#include "RooAbsArg.h"
#include "RooStats/HypoTestInvertorResult.h"

namespace RooStats {

  class HypoTestInvertorPlot : public TNamed {

  public:

    // constructor
    HypoTestInvertorPlot( const char* name, 
			  const char* title,
			  HypoTestInvertorResult* results ) ;

    TGraph* MakePlot() ;

    // destructor
    ~HypoTestInvertorPlot() ;

  private:

    HypoTestInvertorResult* fResults;

  protected:

    ClassDef(HypoTestInvertorPlot,1)  // HypoTestInvertorPlot class

  };
}

#endif
