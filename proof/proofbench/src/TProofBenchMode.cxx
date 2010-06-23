// @(#)root/proofx:$Id$
// Author: Sangsu Ryu 22/06/2010

/*************************************************************************
 * Copyright (C) 1995-2005, Rene Brun and Fons Rademakers.               *
 * All rights reserved.                                                  *
 *                                                                       *
 * For the licensing terms see $ROOTSYS/LICENSE.                         *
 * For the list of contributors see $ROOTSYS/README/CREDITS.             *
 *************************************************************************/

//////////////////////////////////////////////////////////////////////////
//                                                                      //
// TProofBenchMode                                                      //
//                                                                      //
// An abstract class for modes for PROOF benchmark test.                //
// A mode determines how files are generated in the cluster and used    //
// during the PROOF benchmark test.                                     //
//                                                                      //
//////////////////////////////////////////////////////////////////////////

#include "TProofBenchMode.h"

ClassImp(TProofBenchMode)

//______________________________________________________________________________
TProofBenchMode::~TProofBenchMode()
{
   //destructor
}
