
/**********************************************************************************
 * Project: TMVA - a Root-integrated toolkit for multivariate data analysis       *
 * Package: TMVA                                                                  *
 * Classes: PDEFoamKernel                                                         *
 * Web    : http://tmva.sourceforge.net                                           *
 *                                                                                *
 * Description:                                                                   *
 *      Implementation of trivial PDEFoam kernel                                  *
 *                                                                                *
 * Authors (alphabetical):                                                        *
 *      S. Jadach        - Institute of Nuclear Physics, Cracow, Poland           *
 *      Tancredi Carli   - CERN, Switzerland                                      *
 *      Dominik Dannheim - CERN, Switzerland                                      *
 *      Alexander Voigt  - CERN, Switzerland                                      *
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
// PDEFoamKernel
//
// This class is a trivial PDEFoam kernel estimator, which also acts
// as a general kernel interface for PDEFoam.  The Estimate() function
// returns the cell value, given an event 'txvec'.  Derived classes
// should override this function to build more a sophisitcated kernel
// estimator.
// _____________________________________________________________________

#ifndef ROOT_TMVA_PDEFoamKernel
#include "TMVA/PDEFoamKernel.h"
#endif

ClassImp(TMVA::PDEFoamKernel)

//_____________________________________________________________________
TMVA::PDEFoamKernel::PDEFoamKernel()
   : TObject()
   , fLogger(new MsgLogger("PDEFoamKernel"))
{
   // Default constructor for streamer
}

//_____________________________________________________________________
TMVA::PDEFoamKernel::~PDEFoamKernel()
{
   // Destructor
   if (fLogger != NULL)
      delete fLogger;
}

//_____________________________________________________________________
Float_t TMVA::PDEFoamKernel::Estimate(PDEFoam *foam, std::vector<Float_t> &txvec, ECellValue cv)
{
   // Simple kernel estimator.  It returns the cell value 'cv',
   // corresponding to the event vector 'txvec' (in foam coordinates).
   //
   // Parameters:
   //
   // - foam - the pdefoam to search in
   //
   // - txvec - event vector in foam coordinates [0,1]
   //
   // - cv - cell value to estimate

   if (foam == NULL)
      Log() << kFATAL << "<PDEFoamKernel::Estimate>: PDEFoam not set!" << Endl;

   return foam->GetCellValue(foam->FindCell(txvec), cv);
}
