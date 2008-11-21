// @(#)root/roostats:$Id$

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

#ifndef ROOSTATS_HypoTestResult
#include "RooStats/HypoTestResult.h"
#endif

namespace RooStats {

   class HybridPlot; 

   class HybridResult : public TNamed /* , public HypoTestResult*/ {  /// TO DO: inheritance

   public:

      /// Constructor for HybridResult
      HybridResult(const char *name,const char *title,std::vector<double>& testStat_sb_vals,
                   std::vector<double>& testStat_b_vals);

      /// Destructor of HybridResult
      virtual ~HybridResult();

      void SetDataTestStatistics(double testStat_data_val);

      /// TO DO: use from HypoTestResult
      double CLb();
      double CLsplusb();
      double CLs();

      void Add(HybridResult* other);
      HybridPlot* GetPlot(const char* name,const char* title, int n_bins);
      void PrintMore(const char* options);

   private:
      std::vector<float> fTestStat_b; // results for B-only toy-MC
      std::vector<float> fTestStat_sb; // results for S+B toy-MC
      double fTestStat_data; // results for data

   protected:

      ClassDef(HybridResult,1)   // Class containing the results of the HybridCalculator
   };
}

#endif
