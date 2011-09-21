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
         kReadNothingYet, // data read okay
         kReadEntryNotFound, // the tree entry number does not exist
         kReadIOError // problem reading data
      };

      Bool_t IsValid() const { return fValueAddress && 0 == (int)fSetupStatus && 0 == (int)fReadStatus; }
      ESetupStatus GetSetupStatus() const { return fSetupStatus; }
      EReadStatus GetReadStatus() const { return fReadStatus; }

   protected:
      // Constructor used internally by TTreeReader; use
      // TTreeReader::GetArrayOfMember() instead.
      //
      // Constructs a member data array accessor from the address of the array
      // and the pointer to the Int_t that defined the  number of entries in
      // the array.
      TTreeReaderValuePtrBase():
         fTreeReader(0), fBranch(0), fValueAddress(0), fSetupStatus(kSetupNotSetup),
         fReadStatus(kReadNothingYet)
      {}
      virtual ~TTreeReaderValuePtrBase();

      const char* GetName() const { return fBranchName; }

      TTreeReaderValuePtrBase(TTreeReader& tr, const char* branchname, TDictionary* dict);

      void Init(TTreeReader& tr, TLeaf* leaf, TDictionary* dict);
      void Init(TTreeReader& tr, TBranch* branch, TDictionary* dict);

      void UpdateAddresses();
      EReadStatus LoadEntry();
      TLeaf* GetCounterLeaf(TBranch* branch);
      void* GetAddress();
      TDictionary* GetValueType() const;

      void SetBranchName();

   private:
      TString      fBranchName; // name of branch
      TTreeReader* fTreeReader; // tree reader we belong to
      TBranch*     fBranch; // branch of this value or of its leaf
      void*        fValueAddress; // address of value
      ESetupStatus fSetupStatus; // setup status of this data access
      EReadStatus  fReadStatus; // read status of this data access

      friend class TTreeReader;

      ClassDef(TTreeReaderValuePtrBase, 0);//Base class for accessors to data via TTreeReader
   };

} // namespace ROOT


template <typename T>
class TTreeReaderValuePtr: public ROOT::TTreeReaderValuePtrBase {
public:
   TTreeReaderValuePtr() {}
   TTreeReaderValuePtr(TTreeReader& tr, const char* branchname):
      TTreeReaderValuePtrBase(tr, branchname, TDictionary::GetType(typeid(T))) {}

   T* Get() { return *(T**)GetAddress(); }
   T* operator->() { return Get(); }
   T& operator*() { return *Get(); }

   ClassDefT(TTreeReaderValuePtr, 0);//Accessor to data via TTreeReader
};

#endif // ROOT_TTreeReaderValuePtr
