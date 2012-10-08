// @(#)root/cont:$Id$
// Author: Rene Brun   06/03/95

/*************************************************************************
 * Copyright (C) 1995-2000, Rene Brun and Fons Rademakers.               *
 * All rights reserved.                                                  *
 *                                                                       *
 * For the licensing terms see $ROOTSYS/LICENSE.                         *
 * For the list of contributors see $ROOTSYS/README/CREDITS.             *
 *************************************************************************/

#ifndef ROOT_TArrayF
#define ROOT_TArrayF

//////////////////////////////////////////////////////////////////////////
//                                                                      //
// TArrayF                                                              //
//                                                                      //
// Array of chars or bytes (8 bits per element).                        //
// Class kept for backward compatibility.                               //
//                                                                      //
//////////////////////////////////////////////////////////////////////////

#ifndef ROOT_TArray
#include "TArray.h"
#endif


class TArrayF : public TArrayT<Float_t> {

public:

   TArrayF() : TArrayT<Float_t>() {}
   TArrayF(Int_t n) : TArrayT<Float_t>(n) { }
   TArrayF(Int_t n, const Float_t *array) : TArrayT<Float_t>(n, array) { }
   TArrayF(const TArrayF &rhs) : TArrayT<Float_t>(rhs) { }
   TArrayF    &operator=(const TArrayF &rhs) { TArrayT<Float_t>::operator=(rhs); return *this; }
   virtual    ~TArrayF() { }

   ClassDef(TArrayF,1)
};

inline void TArrayF::Streamer(TBuffer &b) 
{ 
   // Stream a TArrayF object
   TArrayT<Float_t>::Streamer(b);
}

#if defined R__TEMPLATE_OVERLOAD_BUG
template <>
#endif
inline TBuffer &operator>>(TBuffer &buf, TArrayF *&obj)
{
   // Read TArrayF object from buffer.

   obj = (TArrayF *) TArray::ReadArray(buf, TArrayF::Class());
   return buf;
}

#if defined R__TEMPLATE_OVERLOAD_BUG
template <>
#endif
inline TBuffer &operator<<(TBuffer &buf, const TArrayF *obj)
{
   // Write a TArrayF object into buffer
   return buf << (TArray*)obj;
}


#endif
