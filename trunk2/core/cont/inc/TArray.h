// @(#)root/cont:$Id$
// Author: Fons Rademakers   21/10/97

/*************************************************************************
 * Copyright (C) 1995-2000, Rene Brun and Fons Rademakers.               *
 * All rights reserved.                                                  *
 *                                                                       *
 * For the licensing terms see $ROOTSYS/LICENSE.                         *
 * For the list of contributors see $ROOTSYS/README/CREDITS.             *
 *************************************************************************/

#ifndef ROOT_TArray
#define ROOT_TArray


//////////////////////////////////////////////////////////////////////////
//                                                                      //
// TArray                                                               //
//                                                                      //
// Abstract array base class. Used by TArrayC, TArrayS, TArrayI,        //
// TArrayL, TArrayF and TArrayD.                                        //
// Data member is public for historical reasons.                        //
//                                                                      //
//////////////////////////////////////////////////////////////////////////

#ifndef ROOT_Rtypes
#include "Rtypes.h"
#endif
#include <string.h>

class TBuffer;

class TArray {

protected:
   Bool_t        BoundsOk(const char *where, Int_t at) const;
   Bool_t        OutOfBoundsError(const char *where, Int_t i) const;
   TArray(const TArray &a): fN(a.fN) { }
   TArray         &operator=(const TArray &rhs) 
     {if(this!=&rhs) fN = rhs.fN; return *this; }

public:
   Int_t     fN;            //Number of array elements

   TArray(): fN(0) { }
   TArray(Int_t n): fN(n) { }
   virtual        ~TArray() { fN = 0; }

   Int_t          GetSize() const { return fN; }
   virtual void   Set(Int_t n) = 0;

   virtual Double_t GetAt(Int_t i) const = 0;
   virtual void   SetAt(Double_t v, Int_t i) = 0;

   static TArray *ReadArray(TBuffer &b, const TClass *clReq);
   static void    WriteArray(TBuffer &b, const TArray *a);

   friend TBuffer &operator<<(TBuffer &b, const TArray *obj);

   ClassDef(TArray,1)  //Abstract array base class
};

#if defined R__TEMPLATE_OVERLOAD_BUG
template <>
#endif
inline TBuffer &operator>>(TBuffer &buf, TArray *&obj)
{
   // Read TArray object from buffer.

   obj = (TArray *) TArray::ReadArray(buf, TArray::Class());
   return buf;
}

#if defined R__TEMPLATE_OVERLOAD_BUG
template <>
#endif
TBuffer &operator<<(TBuffer &b, const TArray *obj);

inline Bool_t TArray::BoundsOk(const char *where, Int_t at) const
{
   return (at < 0 || at >= fN)
                  ? OutOfBoundsError(where, at)
                  : kTRUE;
}


template<typename T>
class TArrayT : public TArray {
public:
   T* fArray;  //[fN] Array of fN values
   TArrayT() : fArray(NULL) {}
   TArrayT(Int_t n) : fArray(NULL) { if(n > 0) Set(n); }
   TArrayT(Int_t n, const T* array) : fArray(NULL) { Set(n, array); }
   TArrayT(const TArrayT<T>& rhs) : TArray(rhs), fArray(NULL) { Set(rhs.fN, rhs.fArray); }
   TArrayT& operator=(const TArrayT<T>& rhs) { if(this != &rhs) Set(rhs.fN, rhs.fArray); return *this; }
   virtual ~TArrayT() { delete [] fArray; }

   void   AddAt(T value, Int_t i) { 
      if (!BoundsOk("TArrayT::AddAt", i)) return;
      fArray[i] = value;
   }
   T       At(Int_t i) const {
      if (!BoundsOk("TArrayT::At", i)) return T();
      return fArray[i];
   }
   void    Adopt(Int_t n, T* array) {
      if (fArray) delete [] fArray;
      fN = n; fArray = array;
   }

   void     Copy(TArrayT<T> &array) const { array.Set(fN, fArray); }
   const T* GetArray() const { return fArray; }
   T*       GetArray() { return fArray; }
   Double_t GetAt(Int_t i) const { return At(i); }
   Stat_t   GetSum() const { Stat_t sum = 0; for(Int_t i = 0; i < fN; ++i) sum += fArray[i]; return sum; }
   void     Reset(T value = T()) { memset(fArray, value, fN * sizeof(T)); }
   void     Set(Int_t n);
   void     Set(Int_t n, const T* array);
   void     SetAt(Double_t value, Int_t i) { AddAt(T(value), i); }

   T&       operator[](Int_t i) { 
      if (!BoundsOk("TArrayT::operator[]", i)) i = 0; 
      return fArray[i]; 
   }
   T        operator[](Int_t i) const {
      if (!BoundsOk("TArrayT::operator[]", i)) return T(); 
      return fArray[i]; 
   }


   ClassDef(TArrayT,1)
};

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



#ifndef __CINT__
template <typename T>
inline TBuffer& operator>>(TBuffer& buf, TArrayT<T>*& obj)
{
   // Read TArrayT object from buffer
   obj = (TArrayT<T> *) TArray::ReadArray(buf, TArrayT<T>::Class());
   return buf;
}

template <typename T>
inline TBuffer& operator<<(TBuffer& buf, const TArrayT<T>* obj)
{
   // Write a TArrayT object into buffer
   return buf << (TArray*)obj;
}
#else
template TBuffer& operator>>(TBuffer& buf, TArrayT<Char_t>*& obj);
template TBuffer& operator>>(TBuffer& buf, TArrayT<Short_t>*& obj);
template TBuffer& operator>>(TBuffer& buf, TArrayT<Int_t>*& obj);
template TBuffer& operator>>(TBuffer& buf, TArrayT<Long_t>*& obj);
template TBuffer& operator>>(TBuffer& buf, TArrayT<Long64_t>*& obj);
template TBuffer& operator>>(TBuffer& buf, TArrayT<Float_t>*& obj);
template TBuffer& operator>>(TBuffer& buf, TArrayT<Double_t>*& obj);

template TBuffer& operator<<(TBuffer& buf, const TArrayT<Char_t>* obj);
template TBuffer& operator<<(TBuffer& buf, const TArrayT<Short_t>* obj);
template TBuffer& operator<<(TBuffer& buf, const TArrayT<Int_t>* obj);
template TBuffer& operator<<(TBuffer& buf, const TArrayT<Long_t>* obj);
template TBuffer& operator<<(TBuffer& buf, const TArrayT<Long64_t>* obj);
template TBuffer& operator<<(TBuffer& buf, const TArrayT<Float_t>* obj);
template TBuffer& operator<<(TBuffer& buf, const TArrayT<Double_t>* obj);
#endif // __CINT__


#endif // ROOT_TArray

