// @(#)root/mathcore:$Id: SparseData.h 29267 2009-06-30 10:16:44Z moneta $
// Author: David Gonzalez Maline Wed Aug 28 15:23:43 2009

/**********************************************************************
 *                                                                    *
 * Copyright (c) 2006  LCG ROOT Math Team, CERN/PH-SFT                *
 *                                                                    *
 *                                                                    *
 **********************************************************************/

// Header file for class SparseData

#ifndef ROOT_Fit_SparseData
#define ROOT_Fit_SparseData

#include <vector>

#ifndef ROOT_Fit_DataVector
#include "Fit/DataVector.h"
#include "Fit/BinData.h"
#endif

#include <iostream>
#include <iterator>
using namespace std;

class TH1;
class THnSparse;

namespace ROOT { 

   namespace Fit { 

      class ProxyListBox;

      class SparseData : public FitData  { 
      public:
         SparseData(TH1* h);
         SparseData(THnSparse* h);
         ~SparseData();

         unsigned int NPoints() const;
         unsigned int NDim() const;

         void Add(std::vector<double>& min, std::vector<double>& max, 
                  const double content, const double error = 1.0);
         
         void PrintList() const;

         void GetBinData(BinData&) const;
         void GetBinDataIntegral(BinData&) const;
         void GetBinDataNoZeros(BinData&) const;
         
      private : 
         ProxyListBox *l;
      };
      
   } // end namespace Fit
   
} // end namespace ROOT



#endif /* ROOT_Fit_SparseData */
