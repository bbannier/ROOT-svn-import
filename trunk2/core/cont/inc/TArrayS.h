// @(#)root/cont:$Id$
// Author: Rene Brun   06/03/95

/*************************************************************************
 * Copyright (C) 1995-2000, Rene Brun and Fons Rademakers.               *
 * All rights reserved.                                                  *
 *                                                                       *
 * For the licensing terms see $ROOTSYS/LICENSE.                         *
 * For the list of contributors see $ROOTSYS/README/CREDITS.             *
 *************************************************************************/

#ifndef ROOT_TArrayS
#define ROOT_TArrayS


//////////////////////////////////////////////////////////////////////////
//                                                                      //
// TArrayS                                                              //
//                                                                      //
// Array of shorts (16 bits per element).                               //
//                                                                      //
//////////////////////////////////////////////////////////////////////////

#ifndef ROOT_TArray
#include "TArray.h"
#endif


class TArrayS : public TArrayT<Short_t> {

public:

   TArrayS() : TArrayT<Short_t>() {}
   TArrayS(Int_t n) : TArrayT<Short_t>(n) { }
   TArrayS(Int_t n, const Short_t *array) : TArrayT<Short_t>(n, array) { }
   TArrayS(const TArrayS &rhs) : TArrayT<Short_t>(rhs) { }
   TArrayS    &operator=(const TArrayS &rhs) { TArrayT<Short_t>::operator=(rhs); return *this; }
   virtual    ~TArrayS() { }

   ClassDef(TArrayS,1) 
};

inline void TArrayS::Streamer(TBuffer &b) 
{ 
   // Stream a TArrayS object
   TArrayT<Short_t>::Streamer(b);
}

#if defined R__TEMPLATE_OVERLOAD_BUG
template <>
#endif
inline TBuffer &operator>>(TBuffer &buf, TArrayS *&obj)
{
   // Read TArrayS object from buffer.

   obj = (TArrayS *) TArray::ReadArray(buf, TArrayS::Class());
   return buf;
}

#if defined R__TEMPLATE_OVERLOAD_BUG
template <>
#endif
inline TBuffer &operator<<(TBuffer &buf, const TArrayS *obj)
{
   // Write a TArrayS object into buffer
   return buf << (TArray*)obj;
}


#endif
