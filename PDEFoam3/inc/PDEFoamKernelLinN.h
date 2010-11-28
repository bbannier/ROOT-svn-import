
/**********************************************************************************
 * Project: TMVA - a Root-integrated toolkit for multivariate data analysis       *
 * Package: TMVA                                                                  *
 * Classes: PDEFoamKernelLinN                                                     *
 * Web    : http://tmva.sourceforge.net                                           *
 *                                                                                *
 * Description:                                                                   *
 *      PDEFoam kernel, which linear weights with the neighbor cells.             *
 *                                                                                *
 * Authors (alphabetical):                                                        *
 *      S. Jadach        - Institute of Nuclear Physics, Cracow, Poland           *
 *      Tancredi Carli   - CERN, Switzerland                                      *
 *      Dominik Dannheim - CERN, Switzerland                                      *
 *      Alexander Voigt  - CERN, Switzerland                                      *
 *                                                                                *
 * Copyright (c) 2010:                                                            *
 *      CERN, Switzerland                                                         *
 *      MPI-K Heidelberg, Germany                                                 *
 *                                                                                *
 * Redistribution and use in source and binary forms, with or without             *
 * modification, are permitted according to the terms listed in LICENSE           *
 * (http://tmva.sourceforge.net/LICENSE)                                          *
 **********************************************************************************/

#ifndef ROOT_TMVA_PDEFoamKernelLinN
#define ROOT_TMVA_PDEFoamKernelLinN

#ifndef ROOT_TMVA_PDEFoam
#include "TMVA/PDEFoam.h"
#endif
#ifndef ROOT_TMVA_PDEFoamKernel
#include "TMVA/PDEFoamKernel.h"
#endif

namespace TMVA {

   class PDEFoamKernelLinN : public PDEFoamKernel {

   protected:
      // helper function, which linear weights with the neighbor cells
      Float_t WeightLinNeighbors( PDEFoam*, std::vector<Float_t>&, ECellValue, Int_t, Int_t, Bool_t );

      // helper function for WeightLinNeighbors()
      Float_t GetAverageNeighborsValue( PDEFoam*, std::vector<Float_t>&, ECellValue );

   public:
      PDEFoamKernelLinN();                 // Constructor
      virtual ~PDEFoamKernelLinN(){};      // Destructor

      // kernel estimator
      virtual Float_t Estimate( PDEFoam*, std::vector<Float_t>&, ECellValue );

      ClassDef(PDEFoamKernelLinN,1) // PDEFoam kernel
   }; // end of PDEFoamKernelLinN
}  // namespace TMVA

#endif
