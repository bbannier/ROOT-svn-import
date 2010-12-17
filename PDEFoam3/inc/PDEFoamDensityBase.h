
/**********************************************************************************
 * Project: TMVA - a Root-integrated toolkit for multivariate data analysis       *
 * Package: TMVA                                                                  *
 * Classes: PDEFoamDensityBase                                                    *
 * Web    : http://tmva.sourceforge.net                                           *
 *                                                                                *
 * Description:                                                                   *
 *      Class PDEFoamDensityBase is an Abstract class representing                *
 *      n-dimensional real positive integrand function                            *
 *      The main function is Density() which provides the event density at a      *
 *      given point during the foam build-up (sampling).                          *
 *                                                                                *
 * Authors (alphabetical):                                                        *
 *      Tancredi Carli   - CERN, Switzerland                                      *
 *      Dominik Dannheim - CERN, Switzerland                                      *
 *      S. Jadach        - Institute of Nuclear Physics, Cracow, Poland           *
 *      Alexander Voigt  - TU Dresden, Germany                                    *
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

#ifndef ROOT_TMVA_PDEFoamDensityBase
#define ROOT_TMVA_PDEFoamDensityBase

#ifndef ROOT_TObject
#include "TObject.h"
#endif

#ifndef ROOT_TMVA_BinarySearchTree
#include "TMVA/BinarySearchTree.h"
#endif
#ifndef ROOT_TMVA_Event
#include "TMVA/Event.h"
#endif
#ifndef ROOT_TMVA_MsgLogger
#include "TMVA/MsgLogger.h"
#endif

namespace TMVA {

   // class definition of underlying density
   class PDEFoamDensityBase : public ::TObject  {

   protected:
      BinarySearchTree *fBst;   // Binary tree to find events within a volume
      std::vector<Double_t> fBox; // range-searching box
      mutable MsgLogger* fLogger;                     //! message logger
      MsgLogger& Log() const { return *fLogger; }

      // calculate volume of fBox
      Double_t GetBoxVolume() const;

   public:
      PDEFoamDensityBase();
      PDEFoamDensityBase(std::vector<Double_t> box);
      PDEFoamDensityBase(const PDEFoamDensityBase&);
      virtual ~PDEFoamDensityBase();

      // fill event into binary search tree
      void FillBinarySearchTree( const Event* ev );

      // set the range-searching box
      void SetBox(std::vector<Double_t> box){ fBox = box; };

      // main function used by PDEFoam
      // returns density at a given point by range searching in BST
      virtual Double_t Density(std::vector<Double_t> &Xarg, Double_t &event_density) = 0;

      ClassDef(PDEFoamDensityBase,1) //Class for density
   };  //end of PDEFoamDensityBase

}  // namespace TMVA

#endif
