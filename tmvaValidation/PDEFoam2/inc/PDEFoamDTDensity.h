
/**********************************************************************************
 * Project: TMVA - a Root-integrated toolkit for multivariate data analysis       *
 * Package: TMVA                                                                  *
 * Classes: PDEFoamDTDensity                                                      *
 * Web    : http://tmva.sourceforge.net                                           *
 *                                                                                *
 * Description:                                                                   *
 *      Class PDEFoamDTDensity is a class representing                            *
 *      n-dimensional real positive integrand function                            *
 *      The main function is Density() which provides the event density at a      *
 *      given point during the foam build-up (sampling).                          *
 *                                                                                *
 * Authors (alphabetical):                                                        *
 *      Tancredi Carli   - CERN, Switzerland                                      *
 *      Dominik Dannheim - CERN, Switzerland                                      *
 *      S. Jadach        - Institute of Nuclear Physics, Cracow, Poland           *
 *      Alexander Voigt  - CERN, Switzerland                                      *
 *      Peter Speckmayer - CERN, Switzerland                                      *
 *                                                                                *
 * Copyright (c) 2008:                                                            *
 *      CERN, Switzerland                                                         *
 *      MPI-K Heidelberg, Germany                                                 *
 *                                                                                *
 * Redistribution and use in source and binary forms, with or without             *
 * modification, are permitted according to the terms listed in LICENSE           *
 * (http://tmva.sourceforge.net/LICENSE)                                          *
 **********************************************************************************/

#ifndef ROOT_TMVA_PDEFoamDTDensity
#define ROOT_TMVA_PDEFoamDTDensity

#include <vector>

#ifndef ROOT_TMVA_PDEFoamDensity
#include "TMVA/PDEFoamDensity.h"
#endif

namespace TMVA {

   // class definition of underlying event density
   class PDEFoamDTDensity : public PDEFoamDensity  {

   public:
      PDEFoamDTDensity();
      PDEFoamDTDensity(const PDEFoam *foam);
      PDEFoamDTDensity(const PDEFoamDTDensity&);
      virtual ~PDEFoamDTDensity(){};

      // main function used by PDEFoam
      // returns event density at a given point by range searching in BST
      virtual Float_t Density(Float_t *Xarg, Float_t &event_density);
      virtual void FillHist(PDEFoamCell*, std::vector<TH1F*>&, std::vector<TH1F*>&, std::vector<TH1F*>&, std::vector<TH1F*>&);

      ClassDef(PDEFoamDTDensity,1) //Class for DT density
   };  //end of PDEFoamDTDensity

}  // namespace TMVA

#endif
