// @(#)root/tree:$Id$
// Author: Axel Naumann, 2010-08-02

/*************************************************************************
 * Copyright (C) 1995-2010, Rene Brun and Fons Rademakers.               *
 * All rights reserved.                                                  *
 *                                                                       *
 * For the licensing terms see $ROOTSYS/LICENSE.                         *
 * For the list of contributors see $ROOTSYS/README/CREDITS.             *
 *************************************************************************/

#ifndef ROOT_TTreeReaderArray
#define ROOT_TTreeReaderArray


////////////////////////////////////////////////////////////////////////////
//                                                                        //
// TTreeReaderArray                                                    //
//                                                                        //
// A simple interface for reading data from trees or chains.              //
//                                                                        //
//                                                                        //
////////////////////////////////////////////////////////////////////////////

#ifndef ROOT_TTreeReaderValue
#include "TTreeReaderValue.h"
#endif

namespace ROOT {
   class TTreeReaderArrayBase: public TTreeReaderValueBase {
   public:
      TTreeReaderArrayBase(TTreeReader* reader, const char* branchname,
                           TDictionary* dict):
         TTreeReaderValueBase(reader, branchname, dict) {}

      size_t GetSize();
      Bool_t IsEmpty() { return !GetSize(); }

   protected:
      void* UntypedAt(size_t idx) const;

      virtual void CreateProxy();

      ClassDefT(TTreeReaderArrayBase, 0);//Accessor to member of an object stored in a collection
   };

} // namespace ROOT

template <typename T>
class TTreeReaderArray: public ROOT::TTreeReaderArrayBase {
public:
   TTreeReaderArray(TTreeReader& tr, const char* branchname):
      TTreeReaderArrayBase(&tr, branchname, TDictionary::GetDictionary(typeid(T*)))
   {
      // Create an array reader of branch "branchname" for TTreeReader "tr".
   }

   T& At(size_t idx) { return ((T*)UntypedAt())[idx]; }
   T& operator[](size_t idx) { return At(idx); }

   ClassDefT(TTreeReaderArray, 0);//Accessor to member of an object stored in a collection
};

#endif // ROOT_TTreeReaderArray
