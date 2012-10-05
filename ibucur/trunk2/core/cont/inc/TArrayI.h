// @(#)root/cont:$Id$
// Author: Rene Brun   06/03/95

/*************************************************************************
 * Copyright (C) 1995-2000, Rene Brun and Fons Rademakers.               *
 * All rights reserved.                                                  *
 *                                                                       *
 * For the licensing terms see $ROOTSYS/LICENSE.                         *
 * For the list of contributors see $ROOTSYS/README/CREDITS.             *
 *************************************************************************/

#ifndef ROOT_TArrayI
#define ROOT_TArrayI


//////////////////////////////////////////////////////////////////////////
//                                                                      //
// TArrayI                                                              //
//                                                                      //
// Array of integers (32 bits per element).                             //
//                                                                      //
//////////////////////////////////////////////////////////////////////////

#ifndef ROOT_TArray
#include "TArray.h"
#endif


class TArrayI : public TArrayT<Int_t> {

public:

   TArrayI() : TArrayT<Int_t>() {}
   TArrayI(Int_t n) : TArrayT<Int_t>(n) { }
   TArrayI(Int_t n, const Int_t *array) : TArrayT<Int_t>(n, array) { }
   TArrayI(const TArrayI &rhs) : TArrayT<Int_t>(rhs) { }
   TArrayI    &operator=(const TArrayI &rhs) { TArrayT<Int_t>::operator=(rhs); return *this; }
   virtual    ~TArrayI() { }

   ClassDef(TArrayI,1)
};

inline void TArrayI::Streamer(TBuffer &b) 
{ 
   // Stream a TArrayI object
   TArrayT<Int_t>::Streamer(b);
}

#if defined R__TEMPLATE_OVERLOAD_BUG
template <>
#endif
inline TBuffer &operator>>(TBuffer &buf, TArrayI *&obj)
{
   // Read TArrayI object from buffer.

   obj = (TArrayI *) TArray::ReadArray(buf, TArrayI::Class());
   return buf;
}

#if defined R__TEMPLATE_OVERLOAD_BUG
template <>
#endif
inline TBuffer &operator<<(TBuffer &buf, const TArrayI *obj)
{
   // Write a TArrayI object into buffer
   return buf << (TArray*)obj;
}


#endif
