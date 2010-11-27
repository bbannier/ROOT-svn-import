
/**********************************************************************************
 * Project: TMVA - a Root-integrated toolkit for multivariate data analysis       *
 * Package: TMVA                                                                  *
 * Classes: PDEFoamMultiTarget                                                    *
 * Web    : http://tmva.sourceforge.net                                           *
 *                                                                                *
 * Description:                                                                   *
 *      Concrete PDEFoamEvent sub-class.  This foam stores the number             *
 *      of events with every cell, as well as the statistical error on            *
 *      the event number.  In addition to PDEFoamEvent it adds a                  *
 *      public function for projecting the target values given an                 *
 *      event with N_variables < dimension of foam                                *
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

#ifndef ROOT_TMVA_PDEFoamMultiTarget
#define ROOT_TMVA_PDEFoamMultiTarget

#ifndef ROOT_TMVA_PDEFoamEvent
#include "TMVA/PDEFoamEvent.h"
#endif

namespace TMVA {

   class PDEFoamMultiTarget : public PDEFoamEvent {

   protected:
      
      PDEFoamMultiTarget(const PDEFoamMultiTarget&); // Copy Constructor  NOT USED

      // ---------- Public functions ----------------------------------
   public:
      PDEFoamMultiTarget();                  // Default constructor (used only by ROOT streamer)
      PDEFoamMultiTarget(const TString&);    // Principal user-defined constructor
      virtual ~PDEFoamMultiTarget(){};       // Default destructor

      // extract the targets from the foam
      std::vector<Float_t> GetTargets( std::vector<Float_t> &vals, ETargetSelection ts );

      // ---------- ROOT class definition
      ClassDef(PDEFoamMultiTarget,1) // Tree of PDEFoamCells
   }; // end of PDEFoamMultiTarget

}  // namespace TMVA

// ---------- Inline functions

#endif
