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

#include "Pointer.h"

#include "Reflex/AttributesExpression.h"
#include "OwnedMember.h"
#include "Function.h"

//-------------------------------------------------------------------------------
Reflex::Internal::Pointer::Pointer(const Type & pointerType,
                                   unsigned int modifiers,
                                   const std::type_info & ti,
                                   const Catalog& catalog)
//-------------------------------------------------------------------------------
: TypeBase(BuildTypeName(pointerType, kQualified).c_str(), modifiers, sizeof(void*), kETPointer, ti, catalog), 
     fPointerType(pointerType) { 
   // Construct the dictionary info for a pointer type.
}


//-------------------------------------------------------------------------------
const std::string&
Reflex::Internal::Pointer::BuildTypeName(std::string& buf,
                                         const Type & pointerType,
                                         unsigned int mod) {
//-------------------------------------------------------------------------------
// Build the pointer type name.
   if (! pointerType.Is(kFunction)) {
      pointerType.Name(buf, mod);
      buf += "*";
      return buf;
   }
   // function pointer and pointer to function members
   else {
      return Function::BuildPointerTypeName(buf, pointerType.ReturnType(), "",
         pointerType.FunctionParameters().Begin(),
         pointerType.FunctionParameters().End(),
         pointerType.Is(kConst) ? kEDConst : 0,
         mod, pointerType.DeclaringScope());
   }
}
