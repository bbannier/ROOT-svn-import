
/**********************************************************************************
 * Project: TMVA - a Root-integrated toolkit for multivariate data analysis       *
 * Package: TMVA                                                                  *
 * Classes: PDEFoamEvent                                                          *
 * Web    : http://tmva.sourceforge.net                                           *
 *                                                                                *
 * Description:                                                                   *
 *      Class for PDEFoamEvent object                                             *
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

#ifndef ROOT_TMVA_PDEFoamEvent
#define ROOT_TMVA_PDEFoamEvent

#ifndef ROOT_TMVA_PDEFoam
#include "TMVA/PDEFoam.h"
#endif

namespace TMVA {

   class PDEFoamEvent : public PDEFoam {

   protected:
      
      PDEFoamEvent(const PDEFoamEvent&); // Copy Constructor  NOT USED

      // ---------- Public functions ----------------------------------
   public:
      PDEFoamEvent();                  // Default constructor (used only by ROOT streamer)
      PDEFoamEvent(const TString&);    // Principal user-defined constructor
      virtual ~PDEFoamEvent(){};       // Default destructor

      // function to fill created cell with given value
      virtual void FillFoamCells(const Event* ev, Float_t wt);

      // get cell value stored in a foam cell
      virtual Double_t GetCellValue( PDEFoamCell* cell, ECellValue cv, Int_t idim1, Int_t idim2 );
      using PDEFoam::GetCellValue;

      // ---------- ROOT class definition
      ClassDef(PDEFoamEvent,1) // Tree of PDEFoamCells
   }; // end of PDEFoamEvent

}  // namespace TMVA

// ---------- Inline functions

#endif
