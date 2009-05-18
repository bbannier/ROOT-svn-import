// @(#)root/reflex:$Name:  $:$Id$
// Author: Axel Naumann, 2009

// Copyright CERN, CH-1211 Geneva 23, 2004-2009, All rights reserved.
//
// Permission to use, copy, modify, and distribute this software for any
// purpose is hereby granted without fee, provided that this copyright and
// permissions notice appear in all copies and derivatives.
//
// This software is provided "as is" without express or implied warranty.

#ifndef REFLEX_BUILD
#define REFLEX_BUILD
#endif

#include "ScopeCatalogImpl.h"

#include "Reflex/internal/ScopeName.h"
#include "CatalogImpl.h"
#include "Namespace.h"

//-------------------------------------------------------------------------------
Reflex::Internal::ScopeCatalogImpl::~ScopeCatalogImpl() {
//-------------------------------------------------------------------------------
// Cleanup memory allocations for scopes.

   ScopeVec_t::const_iterator it, itE;
   for ( it = fScopeVec.begin(), itE = fScopeVec.end(); it != itE; ++it ) {
      delete ((ScopeName*)it->Id());
   }
}

//-------------------------------------------------------------------------------
void
Reflex::Internal::ScopeCatalogImpl::Init() {
//-------------------------------------------------------------------------------
// create the global scope and its "declaring scope" as returned by __NIRVANA__().
   fNirvana = new ScopeName("@N@I@R@V@A@N@A@", 0, fCatalog->ThisCatalog());
   fGlobalScope = (new Namespace(fCatalog))->ToScopeName();
}

//-------------------------------------------------------------------------------
void
Reflex::Internal::ScopeCatalogImpl::Add(Reflex::ScopeName& scope) {
//-------------------------------------------------------------------------------
// Add a scope to the map.
   fName2ScopeMap[&scope.Name()] = scope.ThisScope();
   fScopeVec.push_back(scope.ThisScope());
}


//-------------------------------------------------------------------------------
Reflex::Scope
Reflex::Internal::ScopeCatalogImpl::ByName(const std::string & name) const {
//-------------------------------------------------------------------------------
// Lookup a scope by fully qualified name.
   Name2ScopeMap_t::const_iterator it;
   if (name.size()>2 && name[0]==':' && name[1]==':') {
      const std::string & k = name.substr(2);
      it = Name2ScopeMap().find(&k);
   } else {
      it = Name2ScopeMap().find(&name);
   }
   if (it != Name2ScopeMap().end())
      return it->second;
   else {
      // HERE STARTS AN UGLY HACK WHICH HAS TO BE UNDONE ASAP
      // Lookup scope name as typedef-to-scope (i.e. in types)
      // (also remove include Reflex/Type.h)
      Type t = fCatalog->Types().ByName(name);
      if (t && t.IsTypedef()) {
         while (t.IsTypedef()) t = t.ToType();
         if (t.IsClass() || t.IsEnum() || t.IsUnion())
            return t;
      }
   }
   return Dummy::Scope();
   // END OF UGLY HACK
}

//-------------------------------------------------------------------------------
void
Reflex::Internal::ScopeCatalogImpl::Remove(Reflex::ScopeName& scope) {
//-------------------------------------------------------------------------------
// Remove the scope from the list of known scopes.
   Name2ScopeMap_t::iterator iName2ScopeMap = fName2ScopeMap.find(&scope.Name());
   if (iName2ScopeMap != fName2ScopeMap.end()) {
      fName2ScopeMap.erase(iName2ScopeMap);
      fScopeVec.erase(std::find(fScopeVec.begin(), fScopeVec.end(), scope.ThisScope()));
   }
}
