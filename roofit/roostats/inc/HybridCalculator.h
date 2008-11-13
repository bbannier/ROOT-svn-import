// @(#)root/roostats:$Id: $

/*************************************************************************
 * Project: RooStats                                                     *
 * Package: RooFit/RooStats                                              *
 * Authors:                                                              *
 *   Kyle Cranmer, Lorenzo Moneta, Gregory Schott, Wouter Verkerke       *
 *************************************************************************
 * Copyright (C) 1995-2008, Rene Brun and Fons Rademakers.               *
 * All rights reserved.                                                  *
 *                                                                       *
 * For the licensing terms see $ROOTSYS/LICENSE.                         *
 * For the list of contributors see $ROOTSYS/README/CREDITS.             *
 *************************************************************************/

#ifndef ROOSTATS_HybridCalculator
#define ROOSTATS_HybridCalculator

#include "RooStats/HybridResult.h"
#include "RooStats/HypoTestCalculator.h"
#include "RooStats/HypoTestResult.h"

namespace RooStats {
  class HybridCalculator /*: public HypoTestCalculator*/ {  /// TO DO: inheritance

    public:
	/// Constructor for HybridCalculator
	HybridCalculator(const char *name,
		const char *title,
		RooAbsPdf& sb_model,
		RooAbsPdf& b_model,
		RooArgList& observables,
		RooArgSet& parameters,
		RooAbsPdf& prior_pdf);

	/// Destructor of HybridCalculator
	~HybridCalculator();

	void SetTestStatistics(int index);
	void Calculate(RooAbsData& data, unsigned int nToys, bool usePriors);
	void RunToys(unsigned int nToys, bool usePriors); // private?
	void Print(const char* options);

    private:
	const char* _name; /// TO DO: put to TNamed inherited
	const char* _title; /// TO DO: put to TNamed inherited
	RooAbsPdf& _sbModel;
	RooAbsPdf& _bModel;
	RooArgList& _observables;
	RooArgSet& _parameters;
	RooAbsPdf& _priorPdf;
        unsigned int _testStatisticsIdx;

    protected:
	ClassDef(HybridCalculator,1)
  };
}

#endif
