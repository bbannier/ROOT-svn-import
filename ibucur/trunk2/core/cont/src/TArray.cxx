// @(#)root/cont:$Id$
// Author: Fons Rademakers   21/10/97

/*************************************************************************
 * Copyright (C) 1995-2000, Rene Brun and Fons Rademakers.               *
 * All rights reserved.                                                  *
 *                                                                       *
 * For the licensing terms see $ROOTSYS/LICENSE.                         *
 * For the list of contributors see $ROOTSYS/README/CREDITS.             *
 *************************************************************************/

//////////////////////////////////////////////////////////////////////////
//                                                                      //
// TArray                                                               //
//                                                                      //
// Abstract array base class. Used by TArrayC, TArrayS, TArrayI,        //
// TArrayL, TArrayF and TArrayD.                                        //
// Data member is public for historical reasons.                        //
//                                                                      //
//////////////////////////////////////////////////////////////////////////

#include "TArray.h"
#include "TError.h"
#include "TClass.h"
#include "TBuffer.h"


ClassImp(TArray)

//______________________________________________________________________________
Bool_t TArray::OutOfBoundsError(const char *where, Int_t i) const
{
   // Generate an out-of-bounds error. Always returns false.

   ::Error(where, "index %d out of bounds (size: %d, this: 0x%lx)", i, fN, (Long_t)this);
   return kFALSE;
}

//______________________________________________________________________________
TArray *TArray::ReadArray(TBuffer &b, const TClass *clReq)
{
   // Read TArray object from buffer. Simplified version of
   // TBuffer::ReadObject (does not keep track of multiple
   // references to same array).

   R__ASSERT(b.IsReading());

   // Make sure ReadArray is initialized
   b.InitMap();

   // Before reading object save start position
   UInt_t startpos = UInt_t(b.Length());

   UInt_t tag;
   TClass *clRef = b.ReadClass(clReq, &tag);

   TArray *a;
   if (!clRef) {

      a = 0;

   } else {

      a = (TArray *) clRef->New();
      if (!a) {
         ::Error("TArray::ReadArray", "could not create object of class %s",
                 clRef->GetName());
         // Exception
         return 0;
      }

      a->Streamer(b);

      b.CheckByteCount(startpos, tag, clRef);
   }

   return a;
}

//______________________________________________________________________________
void TArray::WriteArray(TBuffer &b, const TArray *a)
{
   // Write TArray object to buffer. Simplified version of
   // TBuffer::WriteObject (does not keep track of multiple
   // references to the same array).

   R__ASSERT(b.IsWriting());

   // Make sure WriteMap is initialized
   b.InitMap();

   if (!a) {

      b << (UInt_t) 0;

   } else {

      // Reserve space for leading byte count
      UInt_t cntpos = UInt_t(b.Length());
      b.SetBufferOffset(Int_t(cntpos+sizeof(UInt_t)));

      TClass *cl = a->IsA();
      b.WriteClass(cl);

      ((TArray *)a)->Streamer(b);

      // Write byte count
      b.SetByteCount(cntpos);
   }
}

//______________________________________________________________________________
TBuffer &operator<<(TBuffer &buf, const TArray *obj)
{
   // Write TArray or derived object to buffer.

   TArray::WriteArray(buf, obj);
   return buf;
}



template class TArrayT<Char_t>;
template class TArrayT<Double_t>;
template class TArrayT<Float_t>;
template class TArrayT<Int_t>;
template class TArrayT<Long_t>;
template class TArrayT<Short_t>;
template class TArrayT<Long64_t>;


templateClassImp(TArrayT);
ClassImp(TArrayT<Char_t>);
ClassImp(TArrayT<Short_t>);
ClassImp(TArrayT<Int_t>);
ClassImp(TArrayT<Long_t>);
ClassImp(TArrayT<Long64_t>);
ClassImp(TArrayT<Float_t>);
ClassImp(TArrayT<Double_t>);

//______________________________________________________________________________
template <typename T>
void TArrayT<T>::Set(Int_t n)
{
   // Set size of this array to n T values.
   // A new array is created, the old contents copied to the new array,
   // then the old array is deleted.
   // This function should not be called if the array was declared via Adopt.

   if (n < 0) return;
   if (n != fN) {
      T* temp = fArray;
      if (n != 0) {
         fArray = new T[n];
         if (n < fN) memcpy(fArray, temp, n * sizeof(T));
         else {
            memcpy(fArray, temp, fN * sizeof(T));
            memset(&fArray[fN], 0, (n-fN) * sizeof(T));
         }
      } else {
         fArray = 0;
      }
      if (fN) delete [] temp;
      fN = n;
   }
}


//______________________________________________________________________________
template <typename T>
void TArrayT<T>::Set(Int_t n, const T* array)
{
   // Set size of this array to n values and set the contents
   // This function should not be called if the array was declared via Adopt.

   if (fArray && fN != n) {
      delete [] fArray;
      fArray = 0;
   }
   fN = n;
   if (fN == 0) return;
   if (array == 0) return;
   if (!fArray) fArray = new T[fN];
   memmove(fArray, array, n * sizeof(T));
}


