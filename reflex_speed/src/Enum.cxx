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
Reflex::Internal::Enum::Enum( const char * enumType,
                          const std::type_info & ti,
                          unsigned int modifiers )
//-------------------------------------------------------------------------------
// Construct the dictionary information for an enum
   : TypeBase( enumType, sizeof(int), ENUM, ti ),
     ScopeBase( enumType, ENUM ),
     fModifiers( modifiers ) {}


//-------------------------------------------------------------------------------
Reflex::Internal::Enum::~Enum() {
//-------------------------------------------------------------------------------
// Destructor for enum dictionary information.
}




//-------------------------------------------------------------------------------
void
Reflex::Internal::Enum::GenerateDict( DictionaryGenerator & generator ) const {
//-------------------------------------------------------------------------------
// Generate Dictionary information about itself.
         
   size_t lastMember = DataMemberSize()-1;

   if( !(DeclaringScope().Is(gNAMESPACE)) ) {  

      generator.AddIntoFree("\n.AddEnum(\"" + Name() + "\", \"");

      for ( size_t i = 0; i < DataMemberSize(); ++i ) {
         DataMemberAt(i).GenerateDict(generator);
         if ( i < lastMember ) generator.AddIntoFree(";");
      }

      generator.AddIntoFree("\",");
      if      ( Is(gPUBLIC))    generator.AddIntoFree("typeid(" + Name(SCOPED) + "), PUBLIC)");
      else if ( Is(gPROTECTED)) generator.AddIntoFree("typeid(Reflex::ProtectedEnum), PROTECTED)");
      else if ( Is(gPRIVATE))   generator.AddIntoFree("typeid(Reflex::PrivateEnum), PRIVATE)");
   }
   else {

      generator.AddIntoInstances("      EnumBuilder(\"" + Name(SCOPED) + "\", typeid(" + Name(SCOPED) + "), PUBLIC)");
      for ( size_t i = 0; i < DataMemberSize(); ++i ) DataMemberAt(i).GenerateDict(generator);
      generator.AddIntoInstances(";\n");

   }   
}

