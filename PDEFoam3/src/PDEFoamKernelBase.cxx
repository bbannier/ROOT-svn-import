// @(#)root/tmva $Id$
// Author: Dominik Dannheim, Alexander Voigt

/**********************************************************************************
 * Project: TMVA - a Root-integrated toolkit for multivariate data analysis       *
 * Package: TMVA                                                                  *
 * Classes: PDEFoamKernelBase                                                     *
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
// PDEFoamKernelBase
//
// This class is the abstract kernel interface for PDEFoam.  Derived
// classes must override the Estimate() function.
// _____________________________________________________________________

#ifndef ROOT_TMVA_PDEFoamKernelBase
#include "TMVA/PDEFoamKernelBase.h"
#endif

ClassImp(TMVA::PDEFoamKernelBase)

//_____________________________________________________________________
TMVA::PDEFoamKernelBase::PDEFoamKernelBase()
   : TObject()
   , fLogger(new MsgLogger("PDEFoamKernelBase"))
{
   // Default constructor for streamer
}

//_____________________________________________________________________
TMVA::PDEFoamKernelBase::PDEFoamKernelBase(const PDEFoamKernelBase &other)
   : TObject()
   , fLogger(new MsgLogger(*other.fLogger))
{
   // Copy constructor
}

//_____________________________________________________________________
TMVA::PDEFoamKernelBase::~PDEFoamKernelBase()
{
   // Destructor
   if (fLogger != NULL)
      delete fLogger;
}
