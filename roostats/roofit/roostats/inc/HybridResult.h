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

   class HybridResult : public TNamed /* , public HypoTestResult*/ {

   public:

      /// Constructor for HybridResult
      HybridResult(const char *name,const char *title,std::vector<double>& testStat_sb_vals,
                   std::vector<double>& testStat_b_vals);

      HybridResult(const char *name,const char *title);

     /// Default constructor for HybridResult
      HybridResult();

      /// Destructor of HybridResult
      virtual ~HybridResult();

      void SetDataTestStatistics(double testStat_data_val);

      void Add(HybridResult* other);
      HybridPlot* GetPlot(const char* name,const char* title, int n_bins);
      void PrintMore(const char* options);

      /// Get test statistics values for the sb model
      std::vector<double> GetTestStat_sb(){return fTestStat_sb;}

      /// Get test statistics values for the b model
      std::vector<double> GetTestStat_b(){return fTestStat_b;}

      /// Get test statistics value for data
      double GetTestStat_data(){ return fTestStat_data;}



      // Return p-value for null hypothesis
      Double_t NullPValue() const  {return 1;};

      // Return p-value for alternate hypothesis
      Double_t AlternatePValue() const {return 1;};





      // Convert  NullPValue into a "confidence level"
      Double_t CLb() const {return (1.-NullPValue());}

      // Convert  AlternatePValue into a "confidence level"
      Double_t CLsplusb()  const {return AlternatePValue();}

      // CLs is simply CLs+b/CLb (not a method, but a quantity)
      Double_t CLs()  const {
        double thisCLb = CLb();
        if (thisCLb==0) {
          std::cout << "Error: Cannot compute CLs because CLb = 0. Returning CLs = -1\n";
          // TO DO: assert!
          return -1;
        }
        double thisCLsb = CLsplusb();
        return thisCLsb/thisCLb;
      }

      // familiar name for the Null p-value in terms of 1-sided Gaussian significance
      Double_t Significance()  const {return RooStats::PValueToSignificance( NullPValue() ); }



   private:
      std::vector<double> fTestStat_b; // vector of results for B-only toy-MC
      std::vector<double> fTestStat_sb; // vector of results for S+B toy-MC
      double fTestStat_data; // results (test statistics) evaluated for data

      mutable bool fComputationsNulDoneFlag; // flag if the fNullPValue computation have been already done or not (ie need to be refreshed)
      mutable bool fComputationsAltDoneFlag; // flag if the fAlternatePValue computation have been already done or not (ie need to be refreshed)
 
      mutable Double_t fNullPValue; // p-value for the null hypothesis (small number means disfavored)
      mutable Double_t fAlternatePValue; // p-value for the alternate hypothesis (small number means disfavored)

   protected:

      ClassDef(HybridResult,1)  // Class containing the results of the HybridCalculator
   };
}

#endif
