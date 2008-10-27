// @(#)root/roostats:$Id: NumberCountingPdfFactory.h 25835 2008-10-15 19:37:19Z moneta $
// Author: Kyle Cranmer   28/07/2008

/*************************************************************************
 * Copyright (C) 1995-2008, Rene Brun and Fons Rademakers.               *
 * All rights reserved.                                                  *
 *                                                                       *
 * For the licensing terms see $ROOTSYS/LICENSE.                         *
 * For the list of contributors see $ROOTSYS/README/CREDITS.             *
 *************************************************************************/

#ifndef RooStats_NumberCountingPdfFactory
#define RooStats_NumberCountingPdfFactory

/////////////////////////////////////////
// NumberCountingPdfFactory
//
// Encapsulates common number counting utilities
/////////////////////////////////////////
#include "Rtypes.h"

#include "RooWorkspace.h"

namespace RooStats{
  class  NumberCountingPdfFactory {

  public:
    // need one for expected and one for observed
    NumberCountingPdfFactory(Double_t*, Double_t*, Double_t*, Int_t );
    ~NumberCountingPdfFactory();
    RooWorkspace* GetWorkspace() const;
    RooAbsPdf* GetPdf() const;
    RooAbsData* GetData() const;

  private:
    RooWorkspace* fWS;

    void FillWorkspace(Double_t*, Double_t*, Double_t*, Int_t );



  };
}

#endif
