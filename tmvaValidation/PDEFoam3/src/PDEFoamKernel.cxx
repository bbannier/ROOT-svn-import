
/**********************************************************************************
 * Project: TMVA - a Root-integrated toolkit for multivariate data analysis       *
 * Package: TMVA                                                                  *
 * Classes: PDEFoamKernel                                                         *
 * Web    : http://tmva.sourceforge.net                                           *
 *                                                                                *
 * Description:                                                                   *
 *      Implementation of PDEFoam kernel interface                                *
 *                                                                                *
 * Authors (alphabetical):                                                        *
 *      S. Jadach        - Institute of Nuclear Physics, Cracow, Poland           *
 *      Tancredi Carli   - CERN, Switzerland                                      *
 *      Dominik Dannheim - CERN, Switzerland                                      *
 *      Alexander Voigt  - TU Dresden, Germany                                    *
 *                                                                                *
 * Copyright (c) 2010:                                                            *
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
// This class is the abstract kernel interface for PDEFoam.  Derived
// classes must override the Estimate() function.
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
