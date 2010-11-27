
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
 * Copyright (c) 2010:                                                            *
 *      CERN, Switzerland                                                         *
 *      MPI-K Heidelberg, Germany                                                 *
 *                                                                                *
 * Redistribution and use in source and binary forms, with or without             *
 * modification, are permitted according to the terms listed in LICENSE           *
 * (http://tmva.sourceforge.net/LICENSE)                                          *
 **********************************************************************************/

#ifndef ROOT_TMVA_PDEFoamKernel
#include "TMVA/PDEFoamKernel.h"
#endif

ClassImp(TMVA::PDEFoamKernel)

//_____________________________________________________________________
TMVA::PDEFoamKernel::PDEFoamKernel()
   : TObject()
   , fPDEFoam(NULL)
   , fLogger(new MsgLogger("PDEFoamKernel"))
{
   // Default constructor for streamer
}

//_____________________________________________________________________
TMVA::PDEFoamKernel::PDEFoamKernel(PDEFoam *foam)
   : TObject()
   , fPDEFoam(foam)
   , fLogger(new MsgLogger("PDEFoamKernel"))
{
   // User constructor
}

//_____________________________________________________________________
TMVA::PDEFoamKernel::~PDEFoamKernel()
{
   // Destructor
   if (fPDEFoam != NULL)
      delete fPDEFoam;
   if (fLogger != NULL)
      delete fLogger;
}

//_____________________________________________________________________
Float_t TMVA::PDEFoamKernel::Estimate(std::vector<Float_t> &txvec, ECellValue cv)
{
   // Simple kernel estimator.  It returns the cell value 'cv',
   // corresponding to the event vector 'txvec' (in foam coordinates).
   //
   // Parameters:
   //
   // - txvec - event vector in foam coordinates [0,1]
   //
   // - cv - cell value to estimate

   if (fPDEFoam == NULL)
      Log() << kFATAL << "<PDEFoamKernel::Estimate>: PDEFoam not set!" << Endl;

   return fPDEFoam->GetCellValue(fPDEFoam->FindCell(txvec), cv);
}
