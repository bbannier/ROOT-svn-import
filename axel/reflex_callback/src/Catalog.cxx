// @(#)root/reflex:$Name:  $:$Id$
// Author: Axel Naumann, 2007

// Copyright CERN, CH-1211 Geneva 23, 2004-2007, All rights reserved.
//
// Permission to use, copy, modify, and distribute this software for any
// purpose is hereby granted without fee, provided that this copyright and
// permissions notice appear in all copies and derivatives.
//
// This software is provided "as is" without express or implied warranty.

#ifndef REFLEX_BUILD
#define REFLEX_BUILD
#endif

#include "Reflex/Catalog.h"
#include "CatalogImpl.h"

//-------------------------------------------------------------------------------
Reflex::Catalog::Catalog()
   : fImpl(&Internal::CatalogImpl::Instance()) {
//-------------------------------------------------------------------------------
// Construct a catalog object pointing to the static implementation object.
}

//-------------------------------------------------------------------------------
Reflex::Catalog::Catalog(const std::string& name)
   : fImpl(new Internal::CatalogImpl(name)) {
//-------------------------------------------------------------------------------
// Construct a new named catalog.
}

//-------------------------------------------------------------------------------
Reflex::Catalog::Catalog(Reflex::Internal::CatalogImpl* impl)
   : fImpl(impl) {
//-------------------------------------------------------------------------------
// Construct a new catalog given an implementation.
// Used internally by CatalogImpl.
}

//-------------------------------------------------------------------------------
Reflex::Catalog
Reflex::Catalog::Instance() {
//-------------------------------------------------------------------------------
// Return the static catalog instance
   static Catalog sCatalog(&Internal::CatalogImpl::Instance());
   return sCatalog;
}


//-------------------------------------------------------------------------------
Reflex::Type
Reflex::Catalog::ByName(const std::string& name) const {
//-------------------------------------------------------------------------------
   if (name[0] == ':' && name[1] == ':')
      return fImpl->Types().ByName(name.substr(2));
   else
      return fImpl->Types().ByName(name);
}

//-------------------------------------------------------------------------------
Reflex::Type
Reflex::Catalog::ByTypeInfo(const std::type_info & ti) const {
//-------------------------------------------------------------------------------
   return fImpl->Types().ByTypeInfo(ti);
}


//-------------------------------------------------------------------------------
Reflex::Scope
Reflex::Catalog::ScopeByName(const std::string& name) const {
//-------------------------------------------------------------------------------
   return fImpl->Scopes().ByName(name);
}


//-------------------------------------------------------------------------------
Reflex::Type_Iterator
Reflex::Catalog::Type_Begin() const {
//-------------------------------------------------------------------------------
// iterator access
   return fImpl->Types().TypeVec().begin();
}


//-------------------------------------------------------------------------------
Reflex::Scope_Iterator
Reflex::Catalog::Scope_Begin() const {
//-------------------------------------------------------------------------------
   return fImpl->Scopes().ScopeVec().begin();
}


//-------------------------------------------------------------------------------
void
Reflex::Catalog::UnregisterTypeCallback(const CallbackBase& cb) const {
//-------------------------------------------------------------------------------
   return fImpl->UnregisterTypeCallback(cb);
}


//-------------------------------------------------------------------------------
void
Reflex::Catalog::UnregisterScopeCallback(const CallbackBase& cb) const {
//-------------------------------------------------------------------------------
   return fImpl->UnregisterScopeCallback(cb);
}


//-------------------------------------------------------------------------------
void
Reflex::Catalog::UnregisterMemberCallback(const CallbackBase& cb) const {
//-------------------------------------------------------------------------------
   return fImpl->UnregisterMemberCallback(cb);
}
