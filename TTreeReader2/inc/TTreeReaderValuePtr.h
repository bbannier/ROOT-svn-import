// @(#)root/tree:$Id$
// Author: Axel Naumann, 2010-08-02

/*************************************************************************
 * Copyright (C) 1995-2010, Rene Brun and Fons Rademakers.               *
 * All rights reserved.                                                  *
 *                                                                       *
 * For the licensing terms see $ROOTSYS/LICENSE.                         *
 * For the list of contributors see $ROOTSYS/README/CREDITS.             *
 *************************************************************************/

#ifndef ROOT_TTreeReaderValuePtr
#define ROOT_TTreeReaderValuePtr


////////////////////////////////////////////////////////////////////////////
//                                                                        //
// TTreeReaderValuePtr                                                    //
//                                                                        //
// A simple interface for reading data from trees or chains.              //
//                                                                        //
//                                                                        //
////////////////////////////////////////////////////////////////////////////

#ifndef ROOT_TString
#include "TString.h"
#endif
#ifndef ROOT_TDictionary
#include "TDictionary.h"
#endif
#ifndef ROOT_TBranchProxy
#include "TBranchProxy.h"
#endif

class TBranch;
class TBranchElement;
class TLeaf;
class TTreeReader;

namespace ROOT {

   class TTreeReaderValuePtrBase: public TObject {
   public:

      // Status flags, 0 is good
      enum ESetupStatus {
         kSetupNotSetup = -7,
         kSetupTreeDestructed = -8,
         kSetupMakeClassModeMismatch = -7, // readers disagree on whether TTree::SetMakeBranch() should be on
         kSetupMissingCounterBranch = -6,
         kSetupMissingBranch = -5,
         kSetupInternalError = -4,
         kSetupMissingCompiledCollectionProxy = -3,
         kSetupMismatch = -2,
         kSetupClassMismatch = -1,
         kSetupMatch = 0,
         kSetupMatchBranch = 0,
         kSetupMatchConversion,
         kSetupMatchConversionCollection,
         kSetupMakeClass,
         kSetupVoidPtr,
         kSetupNoCheck,
         kSetupMatchLeaf
      };
      enum EReadStatus {
         kReadSuccess = 0, // data read okay
         kReadNothingYet, // data now yet accessed
         kReadError // problem reading data
      };

      EReadStatus ProxyRead();

      Bool_t IsValid() const { return fProxy && 0 == (int)fSetupStatus && 0 == (int)fReadStatus; }
      ESetupStatus GetSetupStatus() const { return fSetupStatus; }
      EReadStatus GetReadStatus() const { return fReadStatus; }

   protected:
      TTreeReaderValuePtrBase(TTreeReader* reader = 0, const char* branchname = 0, TDictionary* dict = 0);

      virtual ~TTreeReaderValuePtrBase();

      void CreateProxy();

      void* GetAddress() {
         return fProxy ? fProxy->GetWhere() : 0;
      }

      void MarkTreeReaderUnavailable() { fTreeReader = 0; }

   private:
      TTreeReader* fTreeReader; // tree reader we belong to
      TString      fBranchName; // name of the branch to read data from.
      TDictionary* fDict; // type that the branch should contain
      ROOT::TBranchProxy* fProxy; // proxy for this branch
      ESetupStatus fSetupStatus; // setup status of this data access
      EReadStatus  fReadStatus; // read status of this data access

      ClassDef(TTreeReaderValuePtrBase, 0);//Base class for accessors to data via TTreeReader

      friend class ::TTreeReader;
   };

} // namespace ROOT


template <typename T>
class TTreeReaderValuePtr: public ROOT::TTreeReaderValuePtrBase {
public:
   TTreeReaderValuePtr() {}
   TTreeReaderValuePtr(TTreeReader& tr, const char* branchname):
      TTreeReaderValuePtrBase(&tr, branchname, TDictionary::GetDictionary(typeid(T))) {}

   T* Get() { return *(T**)GetAddress(); }
   T* operator->() { return Get(); }
   T& operator*() { return *Get(); }

   ClassDefT(TTreeReaderValuePtr, 0);//Accessor to data via TTreeReader
};

#endif // ROOT_TTreeReaderValuePtr
