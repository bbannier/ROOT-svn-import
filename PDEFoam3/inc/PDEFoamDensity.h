
/**********************************************************************************
 * Project: TMVA - a Root-integrated toolkit for multivariate data analysis       *
 * Package: TMVA                                                                  *
 * Classes: PDEFoamDensity                                                        *
 * Web    : http://tmva.sourceforge.net                                           *
 *                                                                                *
 * Description:                                                                   *
 *      Class PDEFoamDensity is an Abstract class representing                    *
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
 * Copyright (c) 2008, 2010:                                                      *
 *      CERN, Switzerland                                                         *
 *      MPI-K Heidelberg, Germany                                                 *
 *                                                                                *
 * Redistribution and use in source and binary forms, with or without             *
 * modification, are permitted according to the terms listed in LICENSE           *
 * (http://tmva.sourceforge.net/LICENSE)                                          *
 **********************************************************************************/

#ifndef ROOT_TMVA_PDEFoamDensity
#define ROOT_TMVA_PDEFoamDensity

#ifndef ROOT_TObject
#include "TObject.h"
#endif

#ifndef ROOT_TMVA_BinarySearchTree
#include "TMVA/BinarySearchTree.h"
#endif
#ifndef ROOT_TMVA_Event
#include "TMVA/Event.h"
#endif
#ifndef ROOT_TMVA_PDEFoam
#include "TMVA/PDEFoam.h"
#endif
#ifndef ROOT_TMVA_MsgLogger
#include "TMVA/MsgLogger.h"
#endif

namespace TMVA {

   // class definition of underlying density
   class PDEFoamDensity : public ::TObject  {

   protected:
      BinarySearchTree *fBst;   // Binary tree to find events within a volume
      std::vector<Double_t> fBox; // range-searching box
      mutable MsgLogger* fLogger;                     //! message logger
      MsgLogger& Log() const { return *fLogger; }

      // calculate volume of fBox
      Double_t GetBoxVolume() const;

   public:
      PDEFoamDensity();
      PDEFoamDensity(std::vector<Double_t> box);
      PDEFoamDensity(const PDEFoamDensity&);
      virtual ~PDEFoamDensity();

      // density build-up functions
      void Initialize(); // create and initialize binary search tree
      void FillBinarySearchTree( const Event* ev );

      // main function used by PDEFoam
      // returns density at a given point by range searching in BST
      virtual Double_t Density(std::vector<Double_t> &Xarg, Double_t &event_density) = 0;

      ClassDef(PDEFoamDensity,1) //Class for density
   };  //end of PDEFoamDensity

}  // namespace TMVA

#endif
