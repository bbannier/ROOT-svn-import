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
#endif

#include <iostream>
#include <iterator>
using namespace std;

class TH1;

namespace ROOT { 

   namespace Fit { 

      class ProxyListBox;

      class SparseData : public FitData  { 
      public:
         SparseData(TH1* h);
         ~SparseData();

         void Add(std::vector<double>& min, std::vector<double>& max, 
                  const double content, const double error);
         
         void PrintList();
         
      private : 
         ProxyListBox *l;
      };
      
   } // end namespace Fit
   
} // end namespace ROOT



#endif /* ROOT_Fit_SparseData */
