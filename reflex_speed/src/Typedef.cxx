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

#include "Typedef.h"

#include "Reflex/Tools.h"
#include "OwnedMember.h"

//-------------------------------------------------------------------------------
Reflex::Internal::Typedef::Typedef(const char * typ,
                                const Type & typedefType,
                                ETYPE typeTyp,
                                unsigned int modifiers,
                                const Type & finalType)
//-------------------------------------------------------------------------------
   : TypeBase(typ, modifiers, typedefType.SizeOf(), typeTyp, typedefType.TypeInfo(), finalType),
     fTypedefType(typedefType) { 
   // Construct typedef info.
}
