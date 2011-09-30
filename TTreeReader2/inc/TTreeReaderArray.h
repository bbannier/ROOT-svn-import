// @(#)root/tree:$Id$
// Author: Axel Naumann, 2010-08-02

/*************************************************************************
 * Copyright (C) 1995-2010, Rene Brun and Fons Rademakers.               *
 * All rights reserved.                                                  *
 *                                                                       *
 * For the licensing terms see $ROOTSYS/LICENSE.                         *
 * For the list of contributors see $ROOTSYS/README/CREDITS.             *
 *************************************************************************/

#ifdef ROOT_TTreeReaderArray
#define ROOT_TTreeReaderArray


////////////////////////////////////////////////////////////////////////////
//                                                                        //
// TTreeReaderArray                                                    //
//                                                                        //
// A simple interface for reading data from trees or chains.              //
//                                                                        //
//                                                                        //
////////////////////////////////////////////////////////////////////////////

#ifndef ROOT_TTreeReaderValuePtr
#include "TTreeReaderValuePtr.h"
#endif

namespace ROOT {
   class TTreeReaderArrayBase: public TTreeReaderValuePtrBase {
   public:
      TTreeReaderArrayBase(TTreeReader& tr, const char* branchname,
                           TDictionary* dict);

      Int_t GetSize();
      Bool_t IsEmpty() { return !GetSize(); }

   protected:
      void* Get();

   private:
      TTreeReaderValuePtrBase fSizeValue; // value reader accessing the size

      ClassDefT(TTreeReaderArrayBase, 0);//Accessor to member of an object stored in a collection
   };

} // namespace ROOT

template <typename T>
class TTreeReaderArray: public ROOT::TTreeReaderArrayBase {
public:
   TTreeReaderArray(TTreeReader& tr, const char* branchname):
      TTreeReaderArrayBase(tr, branchname, TDictionary::GetDictionary(typeid(T*)))
   {
      // Create an array reader of branch "branchname" for TTreeReader "tr".
   }

   T& At(Int_t idx) { return ((T*)Get())[idx]; }
   T& operator[](Int_t idx) { return At(idx); }

   ClassDefT(TTreeReaderArray, 0);//Accessor to member of an object stored in a collection
};

#endif // ROOT_TTreeReaderArray
