
/**********************************************************************************
 * Project: TMVA - a Root-integrated toolkit for multivariate data analysis       *
 * Package: TMVA                                                                  *
 * Classes: PDEFoamEventDensity                                                   *
 * Web    : http://tmva.sourceforge.net                                           *
 *                                                                                *
 * Description:                                                                   *
 *      Class PDEFoamEventDensity is a class representing                         *
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

#ifndef ROOT_TMVA_PDEFoamEventDensity
#define ROOT_TMVA_PDEFoamEventDensity

#ifndef ROOT_TMVA_PDEFoamDensity
#include "TMVA/PDEFoamDensity.h"
#endif

namespace TMVA {

   // class definition of underlying event density
   class PDEFoamEventDensity : public PDEFoamDensity  {

   public:
      PDEFoamEventDensity();
      PDEFoamEventDensity(const PDEFoam *foam);
      PDEFoamEventDensity(const PDEFoamEventDensity&);
      virtual ~PDEFoamEventDensity(){};

      // main function used by PDEFoam
      // returns event density at a given point by range searching in BST
      virtual Float_t Density(Float_t *Xarg, Float_t &event_density);

      ClassDef(PDEFoamEventDensity,1) //Class for Event density
   };  //end of PDEFoamEventDensity

}  // namespace TMVA

#endif
