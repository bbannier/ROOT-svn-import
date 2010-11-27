
/**********************************************************************************
 * Project: TMVA - a Root-integrated toolkit for multivariate data analysis       *
 * Package: TMVA                                                                  *
 * Classes: PDEFoamKernel                                                         *
 * Web    : http://tmva.sourceforge.net                                           *
 *                                                                                *
 * Description:                                                                   *
 *      Trivial PDEFoam kernel                                                    *
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

#ifndef ROOT_TMVA_PDEFoamKernel
#define ROOT_TMVA_PDEFoamKernel

#ifndef ROOT_TObject
#include "TObject.h"
#endif

#ifndef ROOT_TMVA_PDEFoam
#include "TMVA/PDEFoam.h"
#endif

namespace TMVA {

   class PDEFoamKernel : public TObject {

   protected:
      mutable MsgLogger* fLogger;  //! message logger

   public:
      PDEFoamKernel();                 // Constructor
      virtual ~PDEFoamKernel();        // Destructor

      // kernel estimator
      virtual Float_t Estimate( PDEFoam*, std::vector<Float_t>&, ECellValue );

      // Message logger
      MsgLogger& Log() const { return *fLogger; }

      ClassDef(PDEFoamKernel,1) // PDEFoam kernel
   }; // end of PDEFoamKernel
}  // namespace TMVA

#endif
