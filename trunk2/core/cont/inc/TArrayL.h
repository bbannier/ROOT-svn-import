// @(#)root/cont:$Id$
// Author: Rene Brun   06/03/95

/*************************************************************************
 * Copyright (C) 1995-2000, Rene Brun and Fons Rademakers.               *
 * All rights reserved.                                                  *
 *                                                                       *
 * For the licensing terms see $ROOTSYS/LICENSE.                         *
 * For the list of contributors see $ROOTSYS/README/CREDITS.             *
 *************************************************************************/

#ifndef ROOT_TArrayL
#define ROOT_TArrayL

//////////////////////////////////////////////////////////////////////////
//                                                                      //
// TArrayL                                                              //
//                                                                      //
// Array of chars or bytes (8 bits per element).                        //
// Class kept for backward compatibility.                               //
//                                                                      //
//////////////////////////////////////////////////////////////////////////

#ifndef ROOT_TArray
#include "TArray.h"
#endif


class TArrayL : public TArrayT<Long_t> {

public:

   TArrayL() : TArrayT<Long_t>() {}
   TArrayL(Int_t n) : TArrayT<Long_t>(n) { }
   TArrayL(Int_t n, const Long_t *array) : TArrayT<Long_t>(n, array) { }
   TArrayL(const TArrayL &rhs) : TArrayT<Long_t>(rhs) { }
   TArrayL    &operator=(const TArrayL &rhs) { TArrayT<Long_t>::operator=(rhs); return *this; }
   virtual    ~TArrayL() { }

   ClassDef(TArrayL,1)
};

inline void TArrayL::Streamer(TBuffer &b) 
{ 
   // Stream a TArrayL object
   TArrayT<Long_t>::Streamer(b);
}

#if defined R__TEMPLATE_OVERLOAD_BUG
template <>
#endif
inline TBuffer &operator>>(TBuffer &buf, TArrayL *&obj)
{
   // Read TArrayL object from buffer.

   obj = (TArrayL *) TArray::ReadArray(buf, TArrayL::Class());
   return buf;
}

#if defined R__TEMPLATE_OVERLOAD_BUG
template <>
#endif
inline TBuffer &operator<<(TBuffer &buf, const TArrayL *obj)
{
   // Write a TArrayL object into buffer
   return buf << (TArray*)obj;
}


#endif
