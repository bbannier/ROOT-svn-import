
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
 *      Foam build-up).  The function PDEFoamEventDensity::Density() does         *
 *      this job.  It                                                             *
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
TMVA::PDEFoamEventDensity::PDEFoamEventDensity(Int_t dim)
   : PDEFoamDensity(dim)
{}

//_____________________________________________________________________
TMVA::PDEFoamEventDensity::PDEFoamEventDensity(const PDEFoamEventDensity &distr)
   : PDEFoamDensity(distr)
{
   // Copy constructor
   Log() << kFATAL << "COPY CONSTRUCTOR NOT IMPLEMENTED" << Endl;
}

//_____________________________________________________________________
Double_t TMVA::PDEFoamEventDensity::Density( const PDEFoam *foam, std::vector<Double_t> &Xarg, Double_t &event_density )
{
   // This function is needed during the foam buildup.  It return the
   // event density within volume (specified by fVolFrac).
   //
   // Parameters:
   //
   // - foam - the PDEFoam
   //
   // - Xarg - event vector (in [fXmin,fXmax])
   //
   // - event_density - here the event density is stored

   if (!foam)
      Log() << kFATAL << "<PDEFoamEventDensity::Density()> Pointer to owner not set!" << Endl;

   if (!fBst)
      Log() << kFATAL << "<PDEFoamEventDensity::Density()> Binary tree not found!"<< Endl;

   //create volume around point to be found
   std::vector<Double_t> lb(fDim);
   std::vector<Double_t> ub(fDim);

   // probevolume relative to hypercube with edge length 1:
   const Double_t probevolume_inv = std::pow((fVolFrac/2), fDim);

   // set upper and lower bound for search volume
   for (Int_t idim = 0; idim < fDim; idim++) {
      Double_t volsize=(foam->GetXmax(idim) 
			- foam->GetXmin(idim)) / fVolFrac;
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
