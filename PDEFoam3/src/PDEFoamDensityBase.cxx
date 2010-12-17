
/**********************************************************************************
 * Project: TMVA - a Root-integrated toolkit for multivariate data analysis       *
 * Package: TMVA                                                                  *
 * Classes: PDEFoamDensityBase                                                    *
 * Web    : http://tmva.sourceforge.net                                           *
 *                                                                                *
 * Description:                                                                   *
 *      This class provides an interface between the Binary search tree           *
 *      and the PDEFoam object.  In order to build-up the foam one needs to       *
 *      calculate the density of events at a given point (sampling during         *
 *      Foam build-up).  The function PDEFoamDensityBase::Density() does this job. It *
 *      uses a binary search tree, filled with training events, in order to       *
 *      provide this density.                                                     *
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

//_____________________________________________________________________
//
// PDEFoamDensityBase
//
// This is an abstract class, which provides an interface for a
// PDEFoam density estimator.  Derived classes have to implement the
// Density(...) function, which returns the density of a certain
// quantity at a given phase-space point during the foam build-up.
//
// Variants of PDEFoamDensityBase are:
//
//   - PDEFoamEventDensity
//   - PDEFoamDiscriminantDensity
//   - PDEFoamTargetDensity
//   - PDEFoamDecisionTreeDensity
//
// Usage:
//
// The user has to instantiate a sub class of PDEFoamDensityBase and set
// the pointer to the owner, which is a PDEFoam object:
//
//   PDEFoamDensityBase *dens = new MyDensity(dim);
//   pdefoam->SetDensity(dens);
//
// Afterwards the binary search tree should be filled with TMVA
// events:
//
//   dens->FillBinarySearchTree(event);
// _____________________________________________________________________

#ifndef ROOT_TMVA_PDEFoamDensityBase
#include "TMVA/PDEFoamDensityBase.h"
#endif

ClassImp(TMVA::PDEFoamDensityBase)

//_____________________________________________________________________
TMVA::PDEFoamDensityBase::PDEFoamDensityBase()
   : TObject(),
     fBst(new TMVA::BinarySearchTree()),
     fBox(std::vector<Double_t>()),
     fLogger(new MsgLogger("PDEFoamDensityBase"))
{}

//_____________________________________________________________________
TMVA::PDEFoamDensityBase::PDEFoamDensityBase(std::vector<Double_t> box)
   : TObject(),
     fBst(new TMVA::BinarySearchTree()),
     fBox(box),
     fLogger(new MsgLogger("PDEFoamDensityBase"))
{
   if (box.size() == 0)
      Log() << kFATAL << "Dimension of PDEFoamDensityBase is zero" << Endl;

   // set periode (number of variables) of binary search tree
   fBst->SetPeriode(box.size());
}

//_____________________________________________________________________
TMVA::PDEFoamDensityBase::~PDEFoamDensityBase()
{
   if (fBst)    delete fBst;
   if (fLogger) delete fLogger;
}

//_____________________________________________________________________
TMVA::PDEFoamDensityBase::PDEFoamDensityBase(const PDEFoamDensityBase &distr)
   : TObject(),
     fBst(distr.fBst),
     fBox(distr.fBox),
     fLogger(new MsgLogger("PDEFoamDensityBase"))
{
   // Copy constructor
   Log() << kFATAL << "COPY CONSTRUCTOR NOT IMPLEMENTED" << Endl;
}

//_____________________________________________________________________
void TMVA::PDEFoamDensityBase::FillBinarySearchTree(const Event* ev)
{
   // This method inserts the given event 'ev' it into the binary
   // search tree.

   if (fBst == NULL)
      Log() << kFATAL << "<PDEFoamDensityBase::FillBinarySearchTree> "
            << "Binary tree is not set!" << Endl;

   // insert into binary search tree
   fBst->Insert(ev);
}

//_____________________________________________________________________
Double_t TMVA::PDEFoamDensityBase::GetBoxVolume() const
{
   // calculate box volume
   Double_t volume = 1.0;
   for (std::vector<Double_t>::const_iterator it = fBox.begin();
        it != fBox.end(); ++it) {
      volume *= *it;
   }
   return volume;
}
