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

#include "Enum.h"

#include "Reflex/Tools.h"
#include "Reflex/EntityProperty.h"
#include "Reflex/DictionaryGenerator.h"

#include <sstream>


//-------------------------------------------------------------------------------
Reflex::Internal::Enum::Enum(const char * enumType,
                             const std::type_info & ti,
                             const Catalog& catalog,
                             unsigned int modifiers)
//-------------------------------------------------------------------------------
// Construct the dictionary information for an enum
: ScopedType(enumType, modifiers, sizeof(int), kEnum, ti, catalog) {}


//-------------------------------------------------------------------------------
void
Reflex::Internal::Enum::GenerateDict(DictionaryGenerator & generator) const {
//-------------------------------------------------------------------------------
// Generate Dictionary information about itself.

   std::string name;
   if (!(DeclaringScope().Is(gNamespace))) {  

      generator.AddIntoFree("\n.AddEnum(\"" + Name(name) + "\", \"");

      for (OrdMemberCont_t::iterator i = DataMembers().Begin(); i; ++i) {
         i->GenerateDict(generator);
      }
      generator.AddIntoFree(";");

      generator.AddIntoFree("\",");
      name.clear();
      if      (Is(gPublic))    generator.AddIntoFree("typeid(" + Name(name, kScoped) + "), kPublic)");
      else if (Is(gProtected)) generator.AddIntoFree("typeid(Reflex::ProtectedEnum), kProtected)");
      else if (Is(gPrivate))   generator.AddIntoFree("typeid(Reflex::PrivateEnum), kPrivate)");
   }
   else {
      Name(name, kScoped);
      generator.AddIntoInstances("      EnumBuilder(\"" + name + "\", typeid(" + name + "), kPublic)");
      for (OrdMemberCont_t::iterator i = DataMembers().Begin(); i; ++i) {
         i->GenerateDict(generator);
      }
      generator.AddIntoInstances(";\n");

   }   
}

