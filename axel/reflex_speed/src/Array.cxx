// @(#)root/reflex:$Id$
// Author: Stefan Roiser 2004

// Copyright CERN, CH-1211 Geneva 23, 2004-2006, All rights reserved.
//
// Permission to use, copy, modify, and distribute this software for any
// purpose is hereby granted without fee, provided that this copyright and
// permissions notice appear in all copies and derivatives.
//
// This software is provided "as is" without express or implied warranty.

#ifndef REFLEX_BUILD
#define REFLEX_BUILD
#endif

#include "Array.h"

#include "Reflex/Type.h"
#include "Reflex/EntityProperty.h"
#include "OwnedMember.h"

#include <sstream>

//-------------------------------------------------------------------------------
Reflex::Internal::Array::Array(const Type & arrayType,
                               unsigned int modifiers,
                               size_t len,
                               const std::type_info & typeinfo) 
//-------------------------------------------------------------------------------
// Constructs an array type.
   : TypeBase((arrayType.FinalType().Name() + (fNameArraySizeSuffix = BuildTypeNameSuffix(arrayType, len))).c_str(),
              modifiers, len*(arrayType.SizeOf()), kArray, typeinfo),
     fArrayType(arrayType), 
     fLength(len) { }


//-------------------------------------------------------------------------------
const std::string&
Reflex::Internal::Array::Name(std::string& buf, unsigned int mod) const {
//-------------------------------------------------------------------------------
// Return the name of the array type.
   if (fNameArraySizeSuffix.empty())
      fNameArraySizeSuffix = BuildTypeNameSuffix(fArrayType, fLength);
   fArrayType.Name(buf, mod);
   buf += fNameArraySizeSuffix;
   return buf;
}


//-------------------------------------------------------------------------------
std::string
Reflex::Internal::Array::BuildTypeNameSuffix(Type arraytype,
                                             size_t len) {
//-------------------------------------------------------------------------------
// Build an array type name's size suffix, i.e. "[12][10][2]".
   std::ostringstream ost; 
   ost << "[" << len << "]";
   while (arraytype.Is(gArray)) {
      ost << "[" << arraytype.ArrayLength() << "]";
      arraytype = arraytype.ToType();
   }
   return ost.str();
}
