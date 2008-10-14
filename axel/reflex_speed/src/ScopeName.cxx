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

#include "ScopeName.h"

#include "Reflex/Scope.h"
#include "Reflex/Type.h"
#include "Reflex/AttributesExpression.h"
#include "Reflex/Tools.h"

#include "ScopeBase.h"
#include "OwnedMember.h"
#include "CatalogImpl.h"

/*
#include "stl_hash.h"
#include <vector>

//-------------------------------------------------------------------------------
typedef __gnu_cxx::hash_map < const std::string *, Reflex::Scope > Name2Scope_t;
typedef std::vector< Reflex::Scope > ScopeVec_t;

//-------------------------------------------------------------------------------
static Name2Scope_t & sScopes() {
//-------------------------------------------------------------------------------
// Static wrapper around scope map.
   static Name2Scope_t m;
   return m;
}


//-------------------------------------------------------------------------------
static ScopeVec_t & sScopeVec() {
//-------------------------------------------------------------------------------
// Static wrapper around scope vector.
   static ScopeVec_t m;
   return m;
}
*/


//-------------------------------------------------------------------------------
Reflex::Internal::ScopeName::ScopeName(const char* name,
                                       ScopeBase* scopeBase,
                                       const Catalog& catalog)
   : fName(name),
     fScopeBase(scopeBase),
     fCatalog(catalog) {
//-------------------------------------------------------------------------------
// Create the scope name dictionary info.
   fThisScope = new Scope(this);
   /*
   sScopes() [ &fName ] = *fThisScope;
   sScopeVec().push_back(*fThisScope);
   */
   CatalogImpl* cati = fCatalog.Impl();
   cati->Scopes().Add(*this);

   //---Build recursively the declaring scopeNames
   if (fName[0] && fName != "@N@I@R@V@A@N@A@") {
      std::string decl_name = Tools::GetScopeName(fName);
      Scope declScope = cati->Scopes().ByName(decl_name);
      if (! declScope.Id() || declScope.InCatalog() != catalog )
         new ScopeName(decl_name.c_str(), 0, catalog);
   }
}


//-------------------------------------------------------------------------------
Reflex::Internal::ScopeName::~ScopeName() {
//-------------------------------------------------------------------------------
// Destructor.
}


//-------------------------------------------------------------------------------
void
Reflex::Internal::ScopeName::DeleteScope() const {
//-------------------------------------------------------------------------------
// Delete the scope base information.
   delete fScopeBase;
   fScopeBase = 0;
}


//-------------------------------------------------------------------------------
void
Reflex::Internal::ScopeName::HideName() {
//-------------------------------------------------------------------------------
// Append the string " @HIDDEN@" to a scope name.
   if (fName.empty() || fName[fName.length()-1] != '@') {
      fCatalog.Impl()->Scopes().Remove(*this);
      fName += " @HIDDEN@";
      fCatalog.Impl()->Scopes().Add(*this);
   }
}


//-------------------------------------------------------------------------------
Reflex::Scope
Reflex::Internal::ScopeName::ThisScope() const {
//-------------------------------------------------------------------------------
// Return the scope corresponding to this scope.
   return *fThisScope;
}
