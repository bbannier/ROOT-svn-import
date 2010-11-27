
/**********************************************************************************
 * Project: TMVA - a Root-integrated toolkit for multivariate data analysis       *
 * Package: TMVA                                                                  *
 * Classes: PDEFoamEvent                                                          *
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

#ifndef ROOT_TMVA_PDEFoamEvent
#include "TMVA/PDEFoamEvent.h"
#endif

ClassImp(TMVA::PDEFoamEvent)

//_____________________________________________________________________
TMVA::PDEFoamEvent::PDEFoamEvent() 
   : PDEFoam()
{
   // Default constructor for streamer, user should not use it.
}

//_____________________________________________________________________
TMVA::PDEFoamEvent::PDEFoamEvent(const TString& Name)
   : PDEFoam(Name)
{}

//_____________________________________________________________________
TMVA::PDEFoamEvent::PDEFoamEvent(const PDEFoamEvent &From)
   : PDEFoam(From)
{
   // Copy Constructor  NOT IMPLEMENTED (NEVER USED)
   Log() << kFATAL << "COPY CONSTRUCTOR NOT IMPLEMENTED" << Endl;
}

//_____________________________________________________________________
void TMVA::PDEFoamEvent::FillFoamCells(const Event* ev, Float_t wt)
{
   // This function fills an event weight 'wt' into the PDEFoam.  Cell
   // element 0 is filled with the weight 'wt', and element 1 is
   // filled with the squared weight.

   // find corresponding foam cell
   std::vector<Float_t> values  = ev->GetValues();
   std::vector<Float_t> tvalues = VarTransform(values);
   PDEFoamCell *cell = FindCell(tvalues);

   // 0. Element: Number of events
   // 1. Element: RMS
   SetCellElement(cell, 0, GetCellElement(cell, 0) + wt);
   SetCellElement(cell, 1, GetCellElement(cell, 1) + wt*wt);
}

//_____________________________________________________________________
Double_t TMVA::PDEFoamEvent::GetCellValue( PDEFoamCell* cell, ECellValue cv,
					   Int_t idim1, Int_t idim2 )
{
   // Return the discriminant projected onto the dimensions 'dim1',
   // 'dim2'.

   // calculate the projected discriminant
   if (cv == kValue) {

      // get cell position and dimesions
      PDEFoamVect  cellPosi(GetTotDim()), cellSize(GetTotDim());
      cell->GetHcub(cellPosi,cellSize);

      // calculate projected area of cell
      const Double_t area = cellSize[idim1] * cellSize[idim2];
      // calculate projected area of whole foam
      const Double_t foam_area = (fXmax[idim1]-fXmin[idim1])*(fXmax[idim2]-fXmin[idim2]);
      if (area<1e-20){
         Log() << kWARNING << "<Project2>: Warning, cell volume too small --> skiping cell!" << Endl;
         return 0;
      }

      // calc cell entries per projected cell area
      return GetCellValue(cell, kValue)/(area*foam_area);
   } else {
      return GetCellValue(cell, cv);
   }
}
