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
    NumberCountingPdfFactory();
    ~NumberCountingPdfFactory();
    RooWorkspace* GetExpWS(Double_t*, Double_t*, Double_t*, Int_t) ; 
    //  RooWorkspace* GetObsWS(Double_t*, Double_t*, Double_t*, Int_t) ; // needs more thought
    void AddObsData(Double_t* mainMeas, Double_t* bkgMeas, Double_t* db, 
		    Int_t nbins, RooWorkspace* ws);
    void AddObsDataWithSideband(Double_t* mainMeas, Double_t* sideband, 
				Int_t nbins, RooWorkspace* ws);

  private:
    //    RooWorkspace* fWS;  //avoiding having  this class keep state, really use as a factory

    void AddExpData(Double_t*, Double_t*, Double_t*, Int_t, RooWorkspace* );
    //    void AddObsData(Double_t*, Double_t*, Double_t*, Int_t, RooWorkspace* ); // needs more thought
    void AddPdf(Double_t*, Double_t*, Double_t*, Int_t, RooWorkspace* );


  protected:
     //   ClassDef(NumberCountingPdfFactory,1)

  };
}

#endif
