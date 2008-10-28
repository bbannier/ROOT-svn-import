// @(#)root/reflex:$Name:  $:$Id: Catalog.cxx 24948 2008-07-25 13:27:07Z axel $
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
   if (fImpl) {
      if (name[0] == ':' && name[1] == ':')
         return fImpl->Types().ByName(name.substr(2));
      else
         return fImpl->Types().ByName(name);
   }
   return Type();
}

//-------------------------------------------------------------------------------
Reflex::Type
Reflex::Catalog::ByTypeInfo(const std::type_info & ti) const {
//-------------------------------------------------------------------------------
   if (fImpl)
      return fImpl->Types().ByTypeInfo(ti);
   return Type();
}


//-------------------------------------------------------------------------------
Reflex::Scope
Reflex::Catalog::ScopeByName(const std::string& name) const {
//-------------------------------------------------------------------------------
   if (fImpl)
      return fImpl->Scopes().ByName(name);
   return Scope();
}


//-------------------------------------------------------------------------------
const Reflex::Container<Reflex::Type>
Reflex::Catalog::Types() const {
//-------------------------------------------------------------------------------
// iterator access
   if (fImpl)
      return fImpl->Types().All();
   return Dummy::EmptyContainer();
}


//-------------------------------------------------------------------------------
const Reflex::Container<Reflex::Scope>
Reflex::Catalog::Scopes() const {
//-------------------------------------------------------------------------------
   if (fImpl)
      return fImpl->Scopes().All();
   return Dummy::EmptyContainer();
}


//-------------------------------------------------------------------------------
Reflex::Scope
Reflex::Catalog::GlobalScope() const {
//-------------------------------------------------------------------------------
// shortcut access to the global scope
   if (fImpl)
      return fImpl->Scopes().GlobalScope();
   return Scope();
}


//-------------------------------------------------------------------------------
// shortcut access:
Reflex::Scope
Reflex::Catalog::__NIRVANA__() const {
//-------------------------------------------------------------------------------
   if (fImpl)
      return fImpl->Scopes().__NIRVANA__();
   return Scope();
}


//-------------------------------------------------------------------------------
Reflex::Type
Reflex::Catalog::Get(EFUNDAMENTALTYPE etype) {
//-------------------------------------------------------------------------------
   return Internal::CatalogImpl::Instance().Types().Get(etype);
}
