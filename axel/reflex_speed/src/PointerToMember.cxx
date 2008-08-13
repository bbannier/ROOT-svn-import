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

#include "PointerToMember.h"

#include "OwnedMember.h"
#include "Function.h"

//-------------------------------------------------------------------------------
Reflex::Internal::PointerToMember::PointerToMember(const Type & pointerToMemberType,
                                                   unsigned int modifiers,
                                                   const Scope & pointerToMemberScope,
                                                   const std::type_info & ti,
                                                   const Catalog& catalog) 
//------------------------------------------------------------------------------- 
   : TypeBase(BuildTypeName(pointerToMemberType, pointerToMemberScope).c_str(),
              modifiers, sizeof(void*), kPointerToMember, ti, catalog),
     fPointerToMemberType(pointerToMemberType),
     fPointerToMemberScope(pointerToMemberScope) {
   // Construct dictionary info for a pointer to member type.
}


//-------------------------------------------------------------------------------
const std::string&
Reflex::Internal::PointerToMember::Name(std::string& buf, unsigned int mod) const { 
//-------------------------------------------------------------------------------
// Return the name of the pointer to member type.
   return BuildTypeName(buf, fPointerToMemberType, fPointerToMemberScope, mod);
}


//-------------------------------------------------------------------------------
const std::string&
Reflex::Internal::PointerToMember::BuildTypeName(std::string& buf, const Type & pointerToMemberType,
                                                 const Scope & pointerToMemberScope,
                                                 unsigned int mod) {
//-------------------------------------------------------------------------------
// Build the pointer to member type name.
   if (pointerToMemberType.TypeType() == kFunction) {

      return Function::BuildPointerTypeName(buf, pointerToMemberType.ReturnType(), "",
         pointerToMemberType.FunctionParameters(), pointerToMemberType.Is(gConst) ? kConst : 0,
         mod, pointerToMemberScope);

   }
   pointerToMemberType.Name(buf, kQualified | kScoped);
   buf += " ";
   pointerToMemberScope.Name(buf, mod);
   buf += " ::*";
   return buf;
}
                                                          
