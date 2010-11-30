
/**********************************************************************************
 * Project: TMVA - a Root-integrated toolkit for multivariate data analysis       *
 * Package: TMVA                                                                  *
 * Classes: PDEFoamDiscriminant                                                   *
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

#ifndef ROOT_TMVA_PDEFoamDiscriminant
#include "TMVA/PDEFoamDiscriminant.h"
#endif

ClassImp(TMVA::PDEFoamDiscriminant)

//_____________________________________________________________________
TMVA::PDEFoamDiscriminant::PDEFoamDiscriminant() 
   : PDEFoam()
{
   // Default constructor for streamer, user should not use it.
}

//_____________________________________________________________________
TMVA::PDEFoamDiscriminant::PDEFoamDiscriminant(const TString& Name)
   : PDEFoam(Name)
{}

//_____________________________________________________________________
TMVA::PDEFoamDiscriminant::PDEFoamDiscriminant(const PDEFoamDiscriminant &From)
   : PDEFoam(From)
{
   // Copy Constructor  NOT IMPLEMENTED (NEVER USED)
   Log() << kFATAL << "COPY CONSTRUCTOR NOT IMPLEMENTED" << Endl;
}

//_____________________________________________________________________
void TMVA::PDEFoamDiscriminant::FillFoamCells(const Event* ev, Float_t wt)
{
   // This function fills an event into the discriminant PDEFoam.  The
   // event weight 'wt' is filled into cell element 0 if the event is
   // of class 0, and filled into cell element 1 otherwise.

   // find corresponding foam cell
   std::vector<Float_t> values  = ev->GetValues();
   std::vector<Float_t> tvalues = VarTransform(values);
   PDEFoamCell *cell = FindCell(tvalues);

   // 0. Element: Number of signal events
   // 1. Element: Number of background events times normalization
   if (ev->GetClass() == 0)
      SetCellElement(cell, 0, GetCellElement(cell, 0) + wt);
   else
      SetCellElement(cell, 1, GetCellElement(cell, 1) + wt);
}

//_____________________________________________________________________
void TMVA::PDEFoamDiscriminant::Finalize()
{
   // Calc discriminator and its error for every cell and save it to
   // the cell.

   // loop over cells
   for (Long_t iCell=0; iCell<=fLastCe; iCell++) {
      if (!(fCells[iCell]->GetStat()))
         continue;

      Float_t N_sig = GetCellElement(fCells[iCell], 0); // get number of signal events
      Float_t N_bg  = GetCellElement(fCells[iCell], 1); // get number of bg events

      if (N_sig<0.) {
         Log() << kWARNING << "Negative number of signal events in cell " << iCell
               << ": " << N_sig << ". Set to 0." << Endl;
         N_sig=0.;
      }
      if (N_bg<0.) {
         Log() << kWARNING << "Negative number of background events in cell " << iCell
               << ": " << N_bg << ". Set to 0." << Endl;
         N_bg=0.;
      }

      // calculate discriminant
      if (N_sig+N_bg > 1e-10){
	 // discriminant
         SetCellElement(fCells[iCell], 0, N_sig/(N_sig+N_bg));
	 // discriminant error
         SetCellElement(fCells[iCell], 1, TMath::Sqrt( Sqr ( N_sig/Sqr(N_sig+N_bg))*N_sig +
                                                       Sqr ( N_bg /Sqr(N_sig+N_bg))*N_bg ) );

      } else {
         SetCellElement(fCells[iCell], 0, 0.5); // set discriminator
         SetCellElement(fCells[iCell], 1, 1. ); // set discriminator error
      }
   }
}
