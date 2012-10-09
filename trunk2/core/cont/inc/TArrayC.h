// @(#)root/cont:$Id$
// Author: Rene Brun   06/03/95

/*************************************************************************
 * Copyright (C) 1995-2000, Rene Brun and Fons Rademakers.               *
 * All rights reserved.                                                  *
 *                                                                       *
 * For the licensing terms see $ROOTSYS/LICENSE.                         *
 * For the list of contributors see $ROOTSYS/README/CREDITS.             *
 *************************************************************************/

#ifndef ROOT_TArrayC
#define ROOT_TArrayC

//////////////////////////////////////////////////////////////////////////
//                                                                      //
// TArrayC                                                              //
//                                                                      //
// Array of chars or bytes (8 bits per element).                        //
// Class kept for backward compatibility.                               //
//                                                                      //
//////////////////////////////////////////////////////////////////////////

#ifndef ROOT_TArray
#include "TArray.h"
#endif


class TArrayC : public TArrayT<Char_t> {

public:

   TArrayC() : TArrayT<Char_t>() {}
   TArrayC(Int_t n) : TArrayT<Char_t>(n) { }
   TArrayC(Int_t n, const Char_t *array) : TArrayT<Char_t>(n, array) { }
   TArrayC(const TArrayC &rhs) : TArrayT<Char_t>(rhs) { }
   TArrayC    &operator=(const TArrayC &rhs) { TArrayT<Char_t>::operator=(rhs); return *this; }
   virtual    ~TArrayC() { }

   ClassDef(TArrayC,2)  //Array of chars
};

inline void TArrayC::Streamer(TBuffer &b) 
{ 
   // Stream a TArrayC object
   TArrayT<Char_t>::Streamer(b);
}

#if defined R__TEMPLATE_OVERLOAD_BUG
template <>
#endif
inline TBuffer &operator>>(TBuffer &buf, TArrayC *&obj)
{
   // Read TArrayC object from buffer.
   obj = (TArrayC *) TArray::ReadArray(buf, TArrayC::Class());
   return buf;
}

#if defined R__TEMPLATE_OVERLOAD_BUG
template <>
#endif
inline TBuffer &operator<<(TBuffer &buf, const TArrayC *obj)
{
   // Write a TArrayC object into buffer
   return buf << (TArray*)obj;
}


#endif
