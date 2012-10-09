// @(#)root/cont:$Id$
// Author: Rene Brun   06/03/95

/*************************************************************************
 * Copyright (C) 1995-2000, Rene Brun and Fons Rademakers.               *
 * All rights reserved.                                                  *
 *                                                                       *
 * For the licensing terms see $ROOTSYS/LICENSE.                         *
 * For the list of contributors see $ROOTSYS/README/CREDITS.             *
 *************************************************************************/

#ifndef ROOT_TArrayL64
#define ROOT_TArrayL64

//////////////////////////////////////////////////////////////////////////
//                                                                      //
// TArrayL64                                                              //
//                                                                      //
// Array of chars or bytes (8 bits per element).                        //
// Class kept for backward compatibility.                               //
//                                                                      //
//////////////////////////////////////////////////////////////////////////

#ifndef ROOT_TArray
#include "TArray.h"
#endif


class TArrayL64 : public TArrayT<Long64_t> {

public:

   TArrayL64() : TArrayT<Long64_t>() {}
   TArrayL64(Int_t n) : TArrayT<Long64_t>(n) { }
   TArrayL64(Int_t n, const Long64_t *array) : TArrayT<Long64_t>(n, array) { }
   TArrayL64(const TArrayL64 &rhs) : TArrayT<Long64_t>(rhs) { }
   virtual ~TArrayL64();

   ClassDef(TArrayL64,2)  //Array of long64s
};

inline void TArrayL64::Streamer(TBuffer &b) 
{ 
   // Stream a TArrayL64 object
   TArrayT<Long64_t>::Streamer(b);
}

#if defined R__TEMPLATE_OVERLOAD_BUG
template <>
#endif
inline TBuffer &operator>>(TBuffer &buf, TArrayL64 *&obj)
{
   // Read TArrayL64 object from buffer.
   obj = (TArrayL64 *) TArray::ReadArray(buf, TArrayL64::Class());
   return buf;
}

#if defined R__TEMPLATE_OVERLOAD_BUG
template <>
#endif
inline TBuffer &operator<<(TBuffer &buf, const TArrayL64 *obj)
{
   // Write a TArrayL64 object into buffer
   return buf << (TArray*)obj;
}


#endif
