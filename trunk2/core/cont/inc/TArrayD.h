// @(#)root/cont:$Id$
// Author: Rene Brun   06/03/95

/*************************************************************************
 * Copyright (C) 1995-2000, Rene Brun and Fons Rademakers.               *
 * All rights reserved.                                                  *
 *                                                                       *
 * For the licensing terms see $ROOTSYS/LICENSE.                         *
 * For the list of contributors see $ROOTSYS/README/CREDITS.             *
 *************************************************************************/

#ifndef ROOT_TArrayD
#define ROOT_TArrayD

//////////////////////////////////////////////////////////////////////////
//                                                                      //
// TArrayD                                                              //
//                                                                      //
// Array of doubles (64 bits per element).                              //
// Class kept for backward compatibility.                               //
//                                                                      //
//////////////////////////////////////////////////////////////////////////

#ifndef ROOT_TArray
#include "TArray.h"
#endif


class TArrayD : public TArrayT<Double_t> {

public:
   TArrayD() : TArrayT<Double_t>() {}
   TArrayD(Int_t n) : TArrayT<Double_t>(n) { }
   TArrayD(Int_t n, const Double_t *array) : TArrayT<Double_t>(n, array) { }
   TArrayD(const TArrayD &rhs) : TArrayT<Double_t>(rhs) { }

   ClassDef(TArrayD,2)  //Array of doubles
};

inline void TArrayD::Streamer(TBuffer &b) 
{ 
   // Stream a TArrayD object
   TArrayT<Double_t>::Streamer(b);
}

#if defined R__TEMPLATE_OVERLOAD_BUG
template <>
#endif
inline TBuffer &operator>>(TBuffer &buf, TArrayD *&obj)
{
   // Read TArrayD object from buffer.
   obj = (TArrayD *) TArray::ReadArray(buf, TArrayD::Class());
   return buf;
}

#if defined R__TEMPLATE_OVERLOAD_BUG
template <>
#endif
inline TBuffer &operator<<(TBuffer &buf, const TArrayD *obj)
{
   // Write a TArrayD object into buffer
   return buf << (TArray*)obj;
}


#endif
