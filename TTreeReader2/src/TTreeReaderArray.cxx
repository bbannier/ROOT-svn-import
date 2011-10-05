// @(#)root/treeplayer:$Id$
// Author: Axel Naumann, 2011-09-28

/*************************************************************************
 * Copyright (C) 1995-2011, Rene Brun and Fons Rademakers and al.        *
 * All rights reserved.                                                  *
 *                                                                       *
 * For the licensing terms see $ROOTSYS/LICENSE.                         *
 * For the list of contributors see $ROOTSYS/README/CREDITS.             *
 *************************************************************************/

#include "TTreeReaderArray.h"
#include "TTreeReaderValue.h"

////////////////////////////////////////////////////////////////////////////////
//                                                                            //
// TTreeReaderArray                                                        //
//                                                                            //
// Extracts array data from a TTree.                                          //
//                                                                            //
//                                                                            //
//                                                                            //
//                                                                            //
//                                                                            //
//                                                                            //
//                                                                            //
//                                                                            //
//                                                                            //
////////////////////////////////////////////////////////////////////////////////

ClassImp(TTreeReaderArrayBase)

//______________________________________________________________________________
size_t ROOT::TTreeReaderArrayBase::GetSize()
{
   // Get the size of the collection.
}

//______________________________________________________________________________
void* ROOT::TTreeReaderArrayBase::UntypedAt(size_t idx)
{
   // Retrieve pointer to element number idx.
}

//______________________________________________________________________________
void ROOT::TTreeReaderArrayBase::CreateProxy()
{
   // Create the proxy object for our branch.
   if (fProxy) {
      Error("CreateProxy()", "Proxy object for branch %s already exists!",
            fBranchName.Data());
      return;
   }
   if (!fTreeReader) {
      Error("CreateProxy()", "TTreeReader object not set / available for branch %s!",
            fBranchName.Data());
      return;
   }
   if (!fDict) {
      TBranch* br = fTreeReader->GetTree()->GetBranch(fBranchName);
      const char* brDataType = "{UNDETERMINED}";
      if (br) {
         TDictionary* dict = 0;
         brDataType = fTreeReader->GetBranchDataType(br, dict);
      }
      Error("CreateProxy()", "The template argument type T of %s accessing branch %s (which contains data of type %s) is not known to ROOT. You will need to create a dictionary for it.",
            IsA()->GetName(), fBranchName.Data(), brDataType);
      return;
   }

   fProxy = fTreeReader->CreateProxy(fBranchName, fDict);
}
