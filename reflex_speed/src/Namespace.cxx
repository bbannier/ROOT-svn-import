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

#include "Namespace.h"
#include "OwnedMember.h"
#include "Reflex/DictionaryGenerator.h"


//-------------------------------------------------------------------------------
Reflex::Internal::Namespace::Namespace(const char * scop, const Catalog& catalog) 
//-------------------------------------------------------------------------------
   : ScopeBase(scop, kETNamespace, catalog) {
   // Create dictionary info for a namespace scope.
}


//-------------------------------------------------------------------------------
Reflex::Internal::Namespace::Namespace(const Catalog& catalog) 
//-------------------------------------------------------------------------------
   : ScopeBase(catalog) {
   // Constructor for global namespace initialization.
}


//-------------------------------------------------------------------------------
void
Reflex::Internal::Namespace::GenerateDict(DictionaryGenerator & generator) const {
//-------------------------------------------------------------------------------
// Generate Dictionary information about itself.

   
   std::string name;
   Name(name);
   if (name != "" && generator.IsNewType((*this))) {
      std::string nameScoped;
      Name(name, kScoped);
      std::stringstream tempcounter;
      tempcounter << generator.fMethodCounter;
      
      generator.fStr_namespaces<<"NamespaceBuilder nsb" + tempcounter.str() + 
         " (\"" << nameScoped << "\");\n" ;
      
      ++generator.fMethodCounter;
   }
      
   
   for (OrdOwnedMemberCont_t::iterator mi = Members().Begin(); mi; ++mi) 
      mi->GenerateDict(generator); // call Members' own gendict
      
   ScopeBase::GenerateDict(generator);
}

