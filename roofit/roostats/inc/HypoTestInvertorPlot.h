// @(#)root/roostats:$Id: SimpleInterval.h 30478 2009-09-25 19:42:07Z schott $
// Author: Kyle Cranmer, Lorenzo Moneta, Gregory Schott, Wouter Verkerke
/*************************************************************************
 * Copyright (C) 1995-2008, Rene Brun and Fons Rademakers.               *
 * All rights reserved.                                                  *
 *                                                                       *
 * For the licensing terms see $ROOTSYS/LICENSE.                         *
 * For the list of contributors see $ROOTSYS/README/CREDITS.             *
 *************************************************************************/

#ifndef ROOSTATS_HypoTestInverterPlot
#define ROOSTATS_HypoTestInverterPlot

#include "TNamed.h"

class TGraph; 


namespace RooStats {

  class HypoTestInverterResult; 

  class HypoTestInverterPlot : public TNamed {

  public:

    // constructor
    HypoTestInverterPlot( const char* name, 
			  const char* title,
			  HypoTestInverterResult* results ) ;

    TGraph* MakePlot() ;

    // destructor
    ~HypoTestInverterPlot() ;

  private:

    HypoTestInverterResult* fResults;

  protected:

    ClassDef(HypoTestInverterPlot,1)  // HypoTestInverterPlot class

  };
}

#endif
