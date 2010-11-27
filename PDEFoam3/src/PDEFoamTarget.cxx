
/**********************************************************************************
 * Project: TMVA - a Root-integrated toolkit for multivariate data analysis       *
 * Package: TMVA                                                                  *
 * Classes: PDEFoamTarget                                                         *
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

#ifndef ROOT_TMath
#include "TMath.h"
#endif

#ifndef ROOT_TMVA_PDEFoamTarget
#include "TMVA/PDEFoamTarget.h"
#endif

ClassImp(TMVA::PDEFoamTarget)

//_____________________________________________________________________
TMVA::PDEFoamTarget::PDEFoamTarget() 
   : PDEFoam()
{
   // Default constructor for streamer, user should not use it.
}

//_____________________________________________________________________
TMVA::PDEFoamTarget::PDEFoamTarget(const TString& Name)
   : PDEFoam(Name)
{}

//_____________________________________________________________________
TMVA::PDEFoamTarget::PDEFoamTarget(const PDEFoamTarget &From)
   : PDEFoam(From)
{
   // Copy Constructor  NOT IMPLEMENTED (NEVER USED)
   Log() << kFATAL << "COPY CONSTRUCTOR NOT IMPLEMENTED" << Endl;
}

//_____________________________________________________________________
void TMVA::PDEFoamTarget::FillFoamCells(const Event* ev, Float_t wt)
{
   // This function fills an event into the discriminant PDEFoam.  The
   // weight 'wt' is filled into cell element 0 if the event is of
   // class 0, and filled into cell element 1 otherwise.

   // find corresponding foam cell
   std::vector<Float_t> values  = ev->GetValues();
   std::vector<Float_t> tvalues = VarTransform(values);
   std::vector<Float_t> targets = ev->GetTargets();
   PDEFoamCell *cell = FindCell(tvalues);

   // 0. Element: Number of events
   // 1. Element: Target 0
   SetCellElement(cell, 0, GetCellElement(cell, 0) + wt);
   SetCellElement(cell, 1, GetCellElement(cell, 1) + wt*targets.at(0));
}

//_____________________________________________________________________
void TMVA::PDEFoamTarget::Finalize()
{
   // Calculate average cell target in every cell and save them to the
   // cell.  Cell element 0 will contain the average target and cell
   // element 1 will contain the error on the target.

   // loop over cells
   for (Long_t iCell=0; iCell<=fLastCe; iCell++) {
      if (!(fCells[iCell]->GetStat()))
         continue;

      Double_t N_ev  = GetCellElement(fCells[iCell], 0); // get number of events
      Double_t tar   = GetCellElement(fCells[iCell], 1); // get sum of targets

      if (N_ev > 1e-20){
         SetCellElement(fCells[iCell], 0, tar/N_ev);  // set average target
         SetCellElement(fCells[iCell], 1, tar/TMath::Sqrt(N_ev)); // set error on average target
      }
      else {
         SetCellElement(fCells[iCell], 0, 0.0 ); // set mean target
         SetCellElement(fCells[iCell], 1, -1  ); // set mean target error
      }
   }
}

//_____________________________________________________________________
Bool_t TMVA::PDEFoamTarget::CellValueIsUndefined( PDEFoamCell* cell )
{
   // Returns true, if the target error equals -1, as set in
   // Finalize() in case of no events in the cell
   return GetCellValue(cell, kValueError) == -1;
}

//_____________________________________________________________________
Double_t TMVA::PDEFoamTarget::GetCellValue(std::vector<Float_t> &xvec, ECellValue cv, PDEFoamKernel *kernel)
{
   // This function finds the cell, which corresponds to the given
   // untransformed event vector 'xvec' and return its value, which is
   // given by the parameter 'cv'.
   //
   // If cv == kValue, it is checked wether the cell value is
   // undefined.  If this is the case, then the mean of the neighbor's
   // target values is returned, using GetAverageNeighborsValue().
   
   std::vector<Float_t> txvec(VarTransform(xvec));
   PDEFoamCell *cell = FindCell(txvec);

   if (!CellValueIsUndefined(cell)) {
      // cell is not empty
      if (kernel == NULL)
	 return GetCellValue(cell, cv);
      else
	 return kernel->Estimate(this, txvec, cv);
   } else
      // cell is empty -> calc average target of neighbor cells
      return GetAverageNeighborsValue(txvec, kValue);
}

//_____________________________________________________________________
Double_t TMVA::PDEFoamTarget::GetAverageNeighborsValue( std::vector<Float_t> &txvec,
							ECellValue cv )
{
   // This function returns the average value 'cv' of only nearest
   // neighbor cells.  It is used in cases, where empty cells shall
   // not be evaluated.
   //
   // Parameters:
   // - txvec - event vector, transformed into foam coordinates [0, 1]
   // - cv - cell value, see definition of ECellValue

   const Double_t xoffset = 1.e-6;
   Double_t norm   = 0; // normalisation
   Double_t result = 0; // return value

   PDEFoamCell *cell = FindCell(txvec); // find cooresponding cell
   PDEFoamVect cellSize(GetTotDim());
   PDEFoamVect cellPosi(GetTotDim());
   cell->GetHcub(cellPosi, cellSize); // get cell coordinates

   // loop over all dimensions and find neighbor cells
   for (Int_t dim=0; dim<GetTotDim(); dim++) {
      std::vector<Float_t> ntxvec(txvec);
      PDEFoamCell* left_cell  = 0; // left cell
      PDEFoamCell* right_cell = 0; // right cell

      // get left cell
      ntxvec[dim] = cellPosi[dim]-xoffset;
      left_cell = FindCell(ntxvec);
      if (!CellValueIsUndefined(left_cell)){
         // if left cell is not empty, take its value
         result += GetCellValue(left_cell, cv);
         norm++;
      }
      // get right cell
      ntxvec[dim] = cellPosi[dim]+cellSize[dim]+xoffset;
      right_cell = FindCell(ntxvec);
      if (!CellValueIsUndefined(right_cell)){
         // if right cell is not empty, take its value
         result += GetCellValue(right_cell, cv);
         norm++;
      }
   }
   if (norm>0)  result /= norm; // calc average target
   else         result = 0;     // return null if all neighbors are empty

   return result;
}
