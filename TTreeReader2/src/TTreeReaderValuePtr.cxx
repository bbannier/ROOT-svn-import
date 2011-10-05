// @(#)root/treeplayer:$Id$
// Author: Axel Naumann, 2011-09-28

/*************************************************************************
 * Copyright (C) 1995-2011, Rene Brun and Fons Rademakers and al.        *
 * All rights reserved.                                                  *
 *                                                                       *
 * For the licensing terms see $ROOTSYS/LICENSE.                         *
 * For the list of contributors see $ROOTSYS/README/CREDITS.             *
 *************************************************************************/

#include "TTreeReaderValuePtr.h"
#include "TTreeReader.h"
#include "TBranchProxyDirector.h"
#include "TTreeProxyGenerator.h"
#include "TTreeReaderValuePtr.h"

////////////////////////////////////////////////////////////////////////////////
//                                                                            //
// TTreeReaderValuePtr                                                        //
//                                                                            //
// Extracts data from a TTree.                                                //
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

ClassImp(TTreeReaderValuePtrBase)

//______________________________________________________________________________
ROOT::TTreeReaderValuePtrBase::TTreeReaderValuePtrBase(TTreeReader* reader /*= 0*/,
                                                       const char* branchname /*= 0*/,
                                                       TDictionary* dict /*= 0*/):
   fTreeReader(reader),
   fBranchName(branchname),
   fDict(dict),
   fProxy(0),
   fSetupStatus(kSetupNotSetup),
   fReadStatus(kReadNothingYet)
{
   // Construct a tree value reader and register it with the reader object.
   if (fTreeReader) fTreeReader->RegisterValueReader(this);
}

//______________________________________________________________________________
ROOT::TTreeReaderValuePtrBase::~TTreeReaderValuePtrBase()
{
   // Unregister from tree reader, cleanup.
   if (fTreeReader) fTreeReader->DeregisterValueReader(this);
}

//______________________________________________________________________________
ROOT::TTreeReaderValuePtrBase::EReadStatus
ROOT::TTreeReaderValuePtrBase::ProxyRead() {
   if (!fProxy) return kReadNothingYet;
   if (fProxy->Read()) {
      fReadStatus = kReadSuccess;
   } else {
      fReadStatus = kReadError;
   }
   return fReadStatus;
}

//______________________________________________________________________________
void ROOT::TTreeReaderValuePtrBase::CreateProxy() {
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
