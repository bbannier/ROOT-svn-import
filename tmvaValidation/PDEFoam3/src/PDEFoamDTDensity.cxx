
/**********************************************************************************
 * Project: TMVA - a Root-integrated toolkit for multivariate data analysis       *
 * Package: TMVA                                                                  *
 * Classes: PDEFoamDTDensity                                                      *
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
// PDEFoamDTDensity
//
// This is a concrete implementation of PDEFoam.  The Density(...)
// function returns allways 0.  The function FillHist() is added,
// which returns all events in a given PDEFoam cell.
// _____________________________________________________________________

#ifndef ROOT_TMVA_PDEFoamDTDensity
#include "TMVA/PDEFoamDTDensity.h"
#endif

ClassImp(TMVA::PDEFoamDTDensity)

//_____________________________________________________________________
TMVA::PDEFoamDTDensity::PDEFoamDTDensity()
   : PDEFoamDensity()
   , fClass(0)
{}

//_____________________________________________________________________
TMVA::PDEFoamDTDensity::PDEFoamDTDensity(Int_t dim, UInt_t cls)
   : PDEFoamDensity(dim)
   , fClass(cls)
{}

//_____________________________________________________________________
TMVA::PDEFoamDTDensity::PDEFoamDTDensity(const PDEFoamDTDensity &distr)
   : PDEFoamDensity(distr)
   , fClass(0)
{
   // Copy constructor
   Log() << kFATAL << "COPY CONSTRUCTOR NOT IMPLEMENTED" << Endl;
}

//_____________________________________________________________________
Double_t TMVA::PDEFoamDTDensity::Density(const PDEFoam *foam, std::vector<Double_t> &Xarg, Double_t &event_density)
{
   // This function is not used in the decision tree like PDEFoam,
   // instead FillHist() is used.
   return 0;
}

//_____________________________________________________________________
void TMVA::PDEFoamDTDensity::FillHist(const PDEFoam *foam, PDEFoamCell* cell, std::vector<TH1F*> &hsig, std::vector<TH1F*> &hbkg, std::vector<TH1F*> &hsig_unw, std::vector<TH1F*> &hbkg_unw)
{
   // fill the given histograms with signal and background events,
   // which are located in the given cell

   if (!foam)
      Log() << kFATAL << "<PDEFoamDistr::FillHist> Pointer to owner not set!" << Endl;

   // get PDEFoam properties
   Int_t Dim = foam->GetTotDim(); // dimension of foam

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
   std::vector<Double_t> lb(Dim); // lower bound
   std::vector<Double_t> ub(Dim); // upper bound
   for (Int_t idim = 0; idim < Dim; idim++) {
      lb[idim] = foam->VarTransformInvers(idim, cellPosi[idim] - std::numeric_limits<float>::epsilon());
      ub[idim] = foam->VarTransformInvers(idim, cellPosi[idim] + cellSize[idim] + std::numeric_limits<float>::epsilon());
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
      hsig.at(idim)->GetXaxis()->SetLimits(foam->VarTransform(idim,xmin.at(idim)), 
					   foam->VarTransform(idim,xmax.at(idim)));
      hbkg.at(idim)->GetXaxis()->SetLimits(foam->VarTransform(idim,xmin.at(idim)), 
					   foam->VarTransform(idim,xmax.at(idim)));
      hsig_unw.at(idim)->GetXaxis()->SetLimits(foam->VarTransform(idim,xmin.at(idim)), 
					       foam->VarTransform(idim,xmax.at(idim)));
      hbkg_unw.at(idim)->GetXaxis()->SetLimits(foam->VarTransform(idim,xmin.at(idim)), 
					       foam->VarTransform(idim,xmax.at(idim)));
      hsig.at(idim)->Reset();
      hbkg.at(idim)->Reset();
      hsig_unw.at(idim)->Reset();
      hbkg_unw.at(idim)->Reset();
   }

   // fill histograms
   for (UInt_t iev=0; iev<nodes.size(); iev++) {
      std::vector<Float_t> ev = nodes.at(iev)->GetEventV();
      Float_t              wt = nodes.at(iev)->GetWeight();
      for (Int_t idim=0; idim<Dim; idim++) {
	 if (nodes.at(iev)->GetClass() == fClass) {
	    hsig.at(idim)->Fill(foam->VarTransform(idim,ev.at(idim)), wt);
	    hsig_unw.at(idim)->Fill(foam->VarTransform(idim,ev.at(idim)), 1);
	 } else {
	    hbkg.at(idim)->Fill(foam->VarTransform(idim,ev.at(idim)), wt);
	    hbkg_unw.at(idim)->Fill(foam->VarTransform(idim,ev.at(idim)), 1);
	 }
      }
   }
}
