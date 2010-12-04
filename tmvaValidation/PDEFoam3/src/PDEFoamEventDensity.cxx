
/**********************************************************************************
 * Project: TMVA - a Root-integrated toolkit for multivariate data analysis       *
 * Package: TMVA                                                                  *
 * Classes: PDEFoamEventDensity                                                   *
 * Web    : http://tmva.sourceforge.net                                           *
 *                                                                                *
 * Description:                                                                   *
 *      The TFDSITR class provides an interface between the Binary search tree    *
 *      and the PDEFoam object.  In order to build-up the foam one needs to       *
 *      calculate the density of events at a given point (sampling during         *
 *      Foam build-up).  The function PDEFoamEventDensity::Density() does this job.  It  *
 *      uses a binary search tree, filled with training events, in order to       *
 *      provide this density.                                                     *
 *                                                                                *
 * Authors (alphabetical):                                                        *
 *      Tancredi Carli   - CERN, Switzerland                                      *
 *      Dominik Dannheim - CERN, Switzerland                                      *
 *      S. Jadach        - Institute of Nuclear Physics, Cracow, Poland           *
 *      Alexander Voigt  - CERN, Switzerland                                      *
 *      Peter Speckmayer - CERN, Switzerland                                      *
 *                                                                                *
 * Copyright (c) 2008, 2010:                                                            *
 *      CERN, Switzerland                                                         *
 *      MPI-K Heidelberg, Germany                                                 *
 *                                                                                *
 * Redistribution and use in source and binary forms, with or without             *
 * modification, are permitted according to the terms listed in LICENSE           *
 * (http://tmva.sourceforge.net/LICENSE)                                          *
 **********************************************************************************/

//_____________________________________________________________________
//
// PDEFoamEventDensity
//
// This is a concrete implementation of PDEFoam.  Density(...)
// estimates the event (weight) density at a given phase-space point
// using range-searching.
// _____________________________________________________________________

#include <cmath>

#ifndef ROOT_TMVA_PDEFoamEventDensity
#include "TMVA/PDEFoamEventDensity.h"
#endif

ClassImp(TMVA::PDEFoamEventDensity)

//_____________________________________________________________________
TMVA::PDEFoamEventDensity::PDEFoamEventDensity()
   : PDEFoamDensity()
{}

//_____________________________________________________________________
TMVA::PDEFoamEventDensity::PDEFoamEventDensity(const PDEFoam *foam)
   : PDEFoamDensity(foam)
{}

//_____________________________________________________________________
TMVA::PDEFoamEventDensity::PDEFoamEventDensity(const PDEFoamEventDensity &distr)
   : PDEFoamDensity(distr)
{
   // Copy constructor
   Log() << kFATAL << "COPY CONSTRUCTOR NOT IMPLEMENTED" << Endl;
}

//_____________________________________________________________________
Double_t TMVA::PDEFoamEventDensity::Density( Double_t *Xarg, Double_t &event_density )
{
   // This function is needed during the foam buildup.  It return the
   // event density within volume (specified by fVolFrac).

   if (!GetPDEFoam())
      Log() << kFATAL << "<PDEFoamEventDensity::Density()> Pointer to owner not set!" << Endl;

   if (!fBst)
      Log() << kFATAL << "<PDEFoamEventDensity::Density()> Binary tree not found!"<< Endl;

   // get PDEFoam properties
   Int_t Dim       = GetPDEFoam()->GetTotDim(); // dimension of foam

   // make the variable Xarg transform, since Foam only knows about x=[0,1]
   // transformation [0, 1] --> [xmin, xmax]
   for (Int_t idim=0; idim<Dim; idim++)
      Xarg[idim] = GetPDEFoam()->VarTransformInvers(idim, Xarg[idim]);

   //create volume around point to be found
   std::vector<Double_t> lb(Dim);
   std::vector<Double_t> ub(Dim);

   // probevolume relative to hypercube with edge length 1:
   const Double_t probevolume_inv = std::pow((fVolFrac/2), Dim);

   // set upper and lower bound for search volume
   for (Int_t idim = 0; idim < Dim; idim++) {
      Double_t volsize=(GetPDEFoam()->GetXmax(idim) 
			- GetPDEFoam()->GetXmin(idim)) / fVolFrac;
      lb[idim] = Xarg[idim] - volsize;
      ub[idim] = Xarg[idim] + volsize;
   }

   TMVA::Volume volume(&lb, &ub);                        // volume to search in
   std::vector<const TMVA::BinarySearchTreeNode*> nodes; // BST nodes found

   // do range searching
   fBst->SearchVolume(&volume, &nodes);

   // store density based on total number of events
   event_density = nodes.size() * probevolume_inv;

   Double_t weighted_count = 0.; // number of events found (sum of weights)
   for (UInt_t j=0; j<nodes.size(); j++)
      weighted_count += (nodes.at(j))->GetWeight();

   // return:  N_total(weighted) / cell_volume
   return (weighted_count+0.1)*probevolume_inv;
}
