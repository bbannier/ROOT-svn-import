// @(#)root/proofx:$Id:$
// Author:

/*************************************************************************
 * Copyright (C) 1995-2005, Rene Brun and Fons Rademakers.               *
 * All rights reserved.                                                  *
 *                                                                       *
 * For the licensing terms see $ROOTSYS/LICENSE.                         *
 * For the list of contributors see $ROOTSYS/README/CREDITS.             *
 *************************************************************************/

#ifndef ROOT_TProofNode
#define ROOT_TProofNode

//////////////////////////////////////////////////////////////////////////
//                                                                      //
// TProofNode                                                           //
//                                                                      //
// PROOF worker node information                                        //
//                                                                      //
//////////////////////////////////////////////////////////////////////////

#ifndef ROOT_TNamed
#include "TNamed.h"
#endif

class TProofNode : public TNamed
{

private:

   Int_t   fPhysRam;   // Physical RAM of this worker node
   Int_t   fNWrks;     // Number of workers on this node

public:

   TProofNode(const char *wn, Int_t ram) : TNamed(wn,""), fPhysRam(ram), fNWrks(1) {}
   virtual ~TProofNode() {}

   void AddWrks(Int_t n = 1) { fNWrks += n; }
   Int_t GetPhysRam() const { return fPhysRam; }
   Int_t GetNWrks() const { return fNWrks; }
   void SetNWrks(Int_t n) { fNWrks = n; }

   ClassDef(TProofNode,0) //PROOF worker node information
};

#endif
