
/**********************************************************************************
 * Project: TMVA - a Root-integrated toolkit for multivariate data analysis       *
 * Package: TMVA                                                                  *
 * Classes: PDEFoamDiscriminant                                                   *
 * Web    : http://tmva.sourceforge.net                                           *
 *                                                                                *
 * Description:                                                                   *
 *    Concrete PDEFoam sub-class.  This foam stores the discriminant D            *
 *    = N_sig / (N_bg + N_sig) with every cell, as well as the                    *
 *    statistical error on the discriminant.                                      *
 *                                                                                *
 * Authors (alphabetical):                                                        *
 *      S. Jadach        - Institute of Nuclear Physics, Cracow, Poland           *
 *      Tancredi Carli   - CERN, Switzerland                                      *
 *      Dominik Dannheim - CERN, Switzerland                                      *
 *      Alexander Voigt  - CERN, Switzerland                                      *
 *                                                                                *
 * Copyright (c) 2008:                                                            *
 *      CERN, Switzerland                                                         *
 *      MPI-K Heidelberg, Germany                                                 *
 *                                                                                *
 * Redistribution and use in source and binary forms, with or without             *
 * modification, are permitted according to the terms listed in LICENSE           *
 * (http://tmva.sourceforge.net/LICENSE)                                          *
 **********************************************************************************/

#ifndef ROOT_TMVA_PDEFoamDiscriminant
#define ROOT_TMVA_PDEFoamDiscriminant

#ifndef ROOT_TMVA_PDEFoam
#include "TMVA/PDEFoam.h"
#endif

namespace TMVA {

   class PDEFoamDiscriminant : public PDEFoam {

   protected:
      
      PDEFoamDiscriminant(const PDEFoamDiscriminant&); // Copy Constructor  NOT USED

      // ---------- Public functions ----------------------------------
   public:
      PDEFoamDiscriminant();                  // Default constructor (used only by ROOT streamer)
      PDEFoamDiscriminant(const TString&);    // Principal user-defined constructor
      virtual ~PDEFoamDiscriminant(){};       // Default destructor

      // function to fill created cell with given value
      virtual void FillFoamCells(const Event* ev, Float_t wt);

      // function to call after foam is grown
      virtual void Finalize();

      // get cell value stored in a foam cell
      virtual Float_t GetCellValue( PDEFoamCell* cell, ECellValue cv, Int_t idim1, Int_t idim2 );
      using PDEFoam::GetCellValue;

      // ---------- ROOT class definition
      ClassDef(PDEFoamDiscriminant,1) // Tree of PDEFoamCells
   }; // end of PDEFoamDiscriminant

}  // namespace TMVA

// ---------- Inline functions

#endif
