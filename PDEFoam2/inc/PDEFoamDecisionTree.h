
/**********************************************************************************
 * Project: TMVA - a Root-integrated toolkit for multivariate data analysis       *
 * Package: TMVA                                                                  *
 * Classes: PDEFoamDecisionTree                                                   *
 * Web    : http://tmva.sourceforge.net                                           *
 *                                                                                *
 * Description:                                                                   *
 *      Class for decision tree like PDEFoam object                               *
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

#ifndef ROOT_TMVA_PDEFoamDecisionTree
#define ROOT_TMVA_PDEFoamDecisionTree

#ifndef ROOT_TMVA_PDEFoam
#include "TMVA/PDEFoam.h"
#endif

namespace TMVA {

   // separation types
   enum EDTSeparation { kFoam, kGiniIndex, kMisClassificationError, 
			kCrossEntropy };

   class PDEFoamDecisionTree : public PDEFoam {

   private:
      EDTSeparation fDTSeparation; // split cells according to certain separation

   protected:

      virtual void Explore(PDEFoamCell *Cell);     // Exploration of the cell
      Float_t GetSeparation(Float_t s, Float_t b); // calculate separation
      
      PDEFoamDecisionTree(const PDEFoamDecisionTree&); // Copy Constructor  NOT USED

   public:
      PDEFoamDecisionTree();               // Default constructor (used only by ROOT streamer)
      PDEFoamDecisionTree(const TString&, EDTSeparation sep=kFoam); // Principal user-defined constructor
      virtual ~PDEFoamDecisionTree();      // Default destructor

      void SetDTSeparation(EDTSeparation new_val){ fDTSeparation = new_val; }

      // ---------- ROOT class definition
      ClassDef(PDEFoamDecisionTree,1) // Decision tree like PDEFoam
   }; // end of PDEFoamDecisionTree

}  // namespace TMVA

#endif
