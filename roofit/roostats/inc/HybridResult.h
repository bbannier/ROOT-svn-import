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

#ifndef ROOSTATS_HybridResult
#define ROOSTATS_HybridResult

#include "RooStats/HypoTestResult.h"
#include "RooStats/HybridPlot.h"

namespace RooStats {

  class HybridResult /*: public HypoTestResult*/ {  /// TO DO: inheritance

    public:
	/// Constructor for HybridResult
	HybridResult(const char *name,const char *title,std::vector<float>& testStat_sb_vals,
                            std::vector<float>& testStat_b_vals,
                            float testStat_data_val );

	/// Destructor of HybridResult
	virtual ~HybridResult();

	/// TO DO: use from HypoTestResult
	double CLb();
	double CLsplusb();
	double CLs();

	void Add(HybridResult* other);
	HybridPlot* GetPlot(const char* name,const char* title, int n_bins);
	void Print(const char* options);

    private:
	const char* _name; /// TO DO: put to inherited (TNamed for write to file)
	const char* _title; /// TO DO: put to inherited (TNamed for write to file)

        std::vector<float> _testStat_b; // results for B-only toy-MC
        std::vector<float> _testStat_sb; // results for S+B toy-MC
        float _testStat_data; // results for data

    protected:
	ClassDef(HybridResult,1)
  };
}

#endif
