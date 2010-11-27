
/**********************************************************************************
 * Project: TMVA - a Root-integrated toolkit for multivariate data analysis       *
 * Package: TMVA                                                                  *
 * Classes: PDEFoamDTDensity                                                   *
 * Web    : http://tmva.sourceforge.net                                           *
 *                                                                                *
 * Description:                                                                   *
 *      The TFDSITR class provides an interface between the Binary search tree    *
 *      and the PDEFoam object.  In order to build-up the foam one needs to       *
 *      calculate the density of events at a given point (sampling during         *
 *      Foam build-up).  The function PDEFoamDTDensity::Density() does this job.  It  *
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

#ifndef ROOT_TMVA_PDEFoamDTDensity
#include "TMVA/PDEFoamDTDensity.h"
#endif

ClassImp(TMVA::PDEFoamDTDensity)

//_____________________________________________________________________
TMVA::PDEFoamDTDensity::PDEFoamDTDensity()
   : PDEFoamDensity()
{}

//_____________________________________________________________________
TMVA::PDEFoamDTDensity::PDEFoamDTDensity(const PDEFoam *foam)
   : PDEFoamDensity(foam)
{}

//_____________________________________________________________________
TMVA::PDEFoamDTDensity::PDEFoamDTDensity(const PDEFoamDTDensity &distr)
   : PDEFoamDensity(distr)
{
   // Copy constructor
   Log() << kFATAL << "COPY CONSTRUCTOR NOT IMPLEMENTED" << Endl;
}

//_____________________________________________________________________
Float_t TMVA::PDEFoamDTDensity::Density( Float_t *Xarg, Float_t &event_density )
{
   // This function is not used in the decision tree like PDEFoam,
   // instead FillHist() is used.
   return 0;
}

//_____________________________________________________________________
void TMVA::PDEFoamDTDensity::FillHist(PDEFoamCell* cell, std::vector<TH1F*> &hsig, std::vector<TH1F*> &hbkg, std::vector<TH1F*> &hsig_unw, std::vector<TH1F*> &hbkg_unw)
{
   // fill the given histograms with signal and background events,
   // which are located in the given cell

   if (!GetPDEFoam())
      Log() << kFATAL << "<PDEFoamDistr::FillHist> Pointer to owner not set!" << Endl;

   // get PDEFoam properties
   Int_t Dim = GetPDEFoam()->GetTotDim(); // dimension of foam

   // sanity check
   if (!cell)
      Log() << kFATAL << "<PDEFoamDistr::FillHist> Null pointer for cell given!" << Endl;
   if (Int_t(hsig.size()) != Dim || Int_t(hbkg.size()) != Dim || 
       Int_t(hsig_unw.size()) != Dim || Int_t(hbkg_unw.size()) != Dim)
      Log() << kFATAL << "<PDEFoamDistr::FillHist> Edge histograms have wrong size!" << Endl;

   // check histograms
   for (Int_t idim=0; idim<Dim; idim++) {
      if (!hsig.at(idim) || !hbkg.at(idim) || 
	  !hsig_unw.at(idim) || !hbkg_unw.at(idim))
	 Log() << kFATAL << "<PDEFoamDistr::FillHist> Histogram not initialized!" << Endl;
   }

   // get cell position and size
   PDEFoamVect  cellSize(Dim);
   PDEFoamVect  cellPosi(Dim);
   cell->GetHcub(cellPosi, cellSize);

   // determine lower and upper cell bound
   std::vector<Float_t> lb(Dim); // lower bound
   std::vector<Float_t> ub(Dim); // upper bound
   for (Int_t idim = 0; idim < Dim; idim++) {
      lb[idim] = GetPDEFoam()->VarTransformInvers(idim, cellPosi[idim] - std::numeric_limits<float>::epsilon());
      ub[idim] = GetPDEFoam()->VarTransformInvers(idim, cellPosi[idim] + cellSize[idim] + std::numeric_limits<float>::epsilon());
   }

   // create TMVA::Volume object needed for searching within the BST
   TMVA::Volume volume(&lb, &ub); // volume to search in
   std::vector<const TMVA::BinarySearchTreeNode*> nodes; // BST nodes found

   // do range searching
   fBst->SearchVolume(&volume, &nodes);

   // calc xmin and xmax of events found in cell
   std::vector<Float_t> xmin(Dim, std::numeric_limits<float>::max());
   std::vector<Float_t> xmax(Dim, -std::numeric_limits<float>::max());
   for (UInt_t iev=0; iev<nodes.size(); iev++) {
      std::vector<Float_t> ev = nodes.at(iev)->GetEventV();
      for (Int_t idim=0; idim<Dim; idim++) {
	 if (ev.at(idim) < xmin.at(idim))  xmin.at(idim) = ev.at(idim);
	 if (ev.at(idim) > xmax.at(idim))  xmax.at(idim) = ev.at(idim);
      }
   }

   // reset histogram ranges
   for (Int_t idim=0; idim<Dim; idim++) {
      hsig.at(idim)->GetXaxis()->SetLimits(GetPDEFoam()->VarTransform(idim,xmin.at(idim)), 
					   GetPDEFoam()->VarTransform(idim,xmax.at(idim)));
      hbkg.at(idim)->GetXaxis()->SetLimits(GetPDEFoam()->VarTransform(idim,xmin.at(idim)), 
					   GetPDEFoam()->VarTransform(idim,xmax.at(idim)));
      hsig_unw.at(idim)->GetXaxis()->SetLimits(GetPDEFoam()->VarTransform(idim,xmin.at(idim)), 
					       GetPDEFoam()->VarTransform(idim,xmax.at(idim)));
      hbkg_unw.at(idim)->GetXaxis()->SetLimits(GetPDEFoam()->VarTransform(idim,xmin.at(idim)), 
					       GetPDEFoam()->VarTransform(idim,xmax.at(idim)));
      hsig.at(idim)->Reset();
      hbkg.at(idim)->Reset();
      hsig_unw.at(idim)->Reset();
      hbkg_unw.at(idim)->Reset();
   }

   // fill histograms
   for (UInt_t iev=0; iev<nodes.size(); iev++) {
      std::vector<Float_t> ev = nodes.at(iev)->GetEventV();
      Float_t              wt = nodes.at(iev)->GetWeight();
      Bool_t           signal = nodes.at(iev)->IsSignal();
      for (Int_t idim=0; idim<Dim; idim++) {
	 if (signal) {
	    hsig.at(idim)->Fill(GetPDEFoam()->VarTransform(idim,ev.at(idim)), wt);
	    hsig_unw.at(idim)->Fill(GetPDEFoam()->VarTransform(idim,ev.at(idim)), 1);
	 } else {
	    hbkg.at(idim)->Fill(GetPDEFoam()->VarTransform(idim,ev.at(idim)), wt);
	    hbkg_unw.at(idim)->Fill(GetPDEFoam()->VarTransform(idim,ev.at(idim)), 1);
	 }
      }
   }
}
