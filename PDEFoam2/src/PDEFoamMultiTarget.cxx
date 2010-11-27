
/**********************************************************************************
 * Project: TMVA - a Root-integrated toolkit for multivariate data analysis       *
 * Package: TMVA                                                                  *
 * Classes: PDEFoamMultiTarget                                                    *
 * Web    : http://tmva.sourceforge.net                                           *
 *                                                                                *
 * Description:                                                                   *
 *      Implementation.                                                           *
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

#ifndef ROOT_TMVA_PDEFoamMultiTarget
#include "TMVA/PDEFoamMultiTarget.h"
#endif

ClassImp(TMVA::PDEFoamMultiTarget)

//_____________________________________________________________________
TMVA::PDEFoamMultiTarget::PDEFoamMultiTarget() 
   : PDEFoamEvent()
{
   // Default constructor for streamer, user should not use it.
}

//_____________________________________________________________________
TMVA::PDEFoamMultiTarget::PDEFoamMultiTarget(const TString& Name)
   : PDEFoamEvent(Name)
{}

//_____________________________________________________________________
TMVA::PDEFoamMultiTarget::PDEFoamMultiTarget(const PDEFoamMultiTarget &From)
   : PDEFoamEvent(From)
{
   // Copy Constructor  NOT IMPLEMENTED (NEVER USED)
   Log() << kFATAL << "COPY CONSTRUCTOR NOT IMPLEMENTED" << Endl;
}

//_____________________________________________________________________
std::vector<Float_t> TMVA::PDEFoamMultiTarget::GetTargets( std::vector<Float_t> &vals, ETargetSelection ts )
{
   // This function is used when the MultiTargetRegression==True option is set.
   // Returns all regression targets, given the event variables 'vals'.
   // Note: number of foam dimensions = number of variables + number of targets
   //
   // Parameters:
   // - vals - event variables (no targets)
   // - ts - method of target selection (Mean or Mpv)

   // checkt whether vals are within foam borders.
   // if not -> push it into foam
   const Float_t xsmall = 1.e-7;
   for (UInt_t l=0; l<vals.size(); l++) {
      if (vals.at(l) <= fXmin[l]){
         vals.at(l) = fXmin[l] + xsmall;
      }
      else if (vals.at(l) >= fXmax[l]){
         vals.at(l) = fXmax[l] - xsmall;
      }
   }

   // transform variables (vals)
   std::vector<Float_t> tvals(VarTransform(vals));
   std::vector<Float_t> target(GetTotDim()-tvals.size(), 0); // returned vector
   std::vector<Float_t> norm(target); // normalisation
   Double_t max_dens = 0.;            // maximum cell density

   // find cells, which fit tvals (no targets)
   std::vector<PDEFoamCell*> cells = FindCells(tvals);
   if (cells.size()<1) return target;

   // loop over all cells found
   std::vector<PDEFoamCell*>::iterator cell_it(cells.begin());
   for (cell_it=cells.begin(); cell_it!=cells.end(); cell_it++){

      // get density of cell
      Double_t cell_volume  = GetCellValue(*cell_it, kCellVolume);
      Double_t cell_density = 0;
      if (cell_volume > 1e-20)
	 cell_density = GetCellValue(*cell_it, kValue) / cell_volume;

      // get cell position and size
      PDEFoamVect  cellPosi(GetTotDim()), cellSize(GetTotDim());
      (*cell_it)->GetHcub(cellPosi, cellSize);

      // loop over all target dimensions, in order to calculate target
      // value
      if (ts==kMean){
         // sum cell density times cell center
         for (UInt_t itar=0; itar<target.size(); itar++){
            UInt_t idim = itar+tvals.size();
            target.at(itar) += cell_density *
               VarTransformInvers(idim, cellPosi[idim]+0.5*cellSize[idim]);
            norm.at(itar) += cell_density;
         } // loop over targets
      } else {
         // get cell center with maximum event density
         if (cell_density > max_dens){
            max_dens = cell_density; // save new max density
            // fill target values
            for (UInt_t itar=0; itar<target.size(); itar++){
               UInt_t idim = itar+tvals.size();
               target.at(itar) =
                  VarTransformInvers(idim, cellPosi[idim]+0.5*cellSize[idim]);
            } // loop over targets
         }
      }
   } // loop over cells

   // normalise mean cell density
   if (ts==kMean){
      for (UInt_t itar=0; itar<target.size(); itar++){
         if (norm.at(itar)>1.0e-15)
            target.at(itar) /= norm.at(itar);
         else
            // normalisation factor is too small -> return approximate
            // target value
            target.at(itar) = (fXmax[itar+tvals.size()]-fXmin[itar+tvals.size()])/2.;
      }
   }

   return target;
}
