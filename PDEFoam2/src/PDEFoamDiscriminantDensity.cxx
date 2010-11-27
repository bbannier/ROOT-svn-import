
/**********************************************************************************
 * Project: TMVA - a Root-integrated toolkit for multivariate data analysis       *
 * Package: TMVA                                                                  *
 * Classes: PDEFoamDiscriminantDensity                                            *
 * Web    : http://tmva.sourceforge.net                                           *
 *                                                                                *
 * Description:                                                                   *
 *      The TFDSITR class provides an interface between the Binary search tree    *
 *      and the PDEFoam object.  In order to build-up the foam one needs to       *
 *      calculate the density of events at a given point (sampling during         *
 *      Foam build-up).  The function PDEFoamDiscriminantDensity::Density() does this job.  It  *
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
 * Copyright (c) 2008:                                                            *
 *      CERN, Switzerland                                                         *
 *      MPI-K Heidelberg, Germany                                                 *
 *                                                                                *
 * Redistribution and use in source and binary forms, with or without             *
 * modification, are permitted according to the terms listed in LICENSE           *
 * (http://tmva.sourceforge.net/LICENSE)                                          *
 **********************************************************************************/

#include <cmath>

#ifndef ROOT_TMVA_PDEFoamDiscriminantDensity
#include "TMVA/PDEFoamDiscriminantDensity.h"
#endif

ClassImp(TMVA::PDEFoamDiscriminantDensity)

//_____________________________________________________________________
TMVA::PDEFoamDiscriminantDensity::PDEFoamDiscriminantDensity()
   : PDEFoamDensity()
{}

//_____________________________________________________________________
TMVA::PDEFoamDiscriminantDensity::PDEFoamDiscriminantDensity(const PDEFoam *foam)
   : PDEFoamDensity(foam)
{}

//_____________________________________________________________________
TMVA::PDEFoamDiscriminantDensity::PDEFoamDiscriminantDensity(const PDEFoamDiscriminantDensity &distr)
   : PDEFoamDensity(distr)
{
   // Copy constructor
   Log() << kFATAL << "COPY CONSTRUCTOR NOT IMPLEMENTED" << Endl;
}

//_____________________________________________________________________
Float_t TMVA::PDEFoamDiscriminantDensity::Density( Float_t *Xarg, Float_t &event_density )
{
   // This function is needed during the foam buildup.  It returns the
   // average target value within volume divided by volume (specified
   // by VolFrac).

   if (!GetPDEFoam())
      Log() << kFATAL << "<PDEFoamDiscriminantDensity::Density()> Pointer to owner not set!" << Endl;

   if (!fBst)
      Log() << kFATAL << "<PDEFoamDiscriminantDensity::Density()> Binary tree not found!"<< Endl;

   // get PDEFoam properties
   Int_t Dim       = GetPDEFoam()->GetTotDim(); // dimension of foam
   Float_t VolFrac = GetPDEFoam()->GetVolumeFraction(); // get fVolFrac

   // make the variable Xarg transform, since Foam only knows about x=[0,1]
   // transformation [0, 1] --> [xmin, xmax]
   for (Int_t idim=0; idim<Dim; idim++)
      Xarg[idim] = GetPDEFoam()->VarTransformInvers(idim, Xarg[idim]);

   //create volume around point to be found
   std::vector<Float_t> lb(Dim);
   std::vector<Float_t> ub(Dim);

   // probevolume relative to hypercube with edge length 1:
   const Float_t probevolume_inv = std::pow((VolFrac/2), Dim);

   // set upper and lower bound for search volume
   for (Int_t idim = 0; idim < Dim; idim++) {
      Float_t volsize=(GetPDEFoam()->GetXmax(idim) 
		       - GetPDEFoam()->GetXmin(idim)) / VolFrac;
      lb[idim] = Xarg[idim] - volsize;
      ub[idim] = Xarg[idim] + volsize;
   }

   TMVA::Volume volume(&lb, &ub);                        // volume to search in
   std::vector<const TMVA::BinarySearchTreeNode*> nodes; // BST nodes found

   // do range searching
   fBst->SearchVolume(&volume, &nodes);

   // store density based on total number of events
   event_density = nodes.size() * probevolume_inv;

   Float_t weighted_count = 0.; // number of events found (sum of weights)
   Float_t N_sig = 0;           // number of signal events found
   // calc number of signal events in nodes
   for (UInt_t j=0; j<nodes.size(); j++) {
      if (nodes.at(j)->IsSignal()) N_sig += nodes.at(j)->GetWeight();
      weighted_count += (nodes.at(j))->GetWeight();
   }

   // return:  (N_sig/N_total) / (cell_volume)
   return (N_sig/(weighted_count+0.1))*probevolume_inv;
}
