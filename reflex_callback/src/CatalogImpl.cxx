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

#include "CatalogImpl.h"

//-------------------------------------------------------------------------------
Reflex::Internal::CatalogImpl&
Reflex::Internal::CatalogImpl::Instance() {
//-------------------------------------------------------------------------------
// Return the global instance of the reflection catalog.
   static CatalogImpl instance;

   return instance;
}

#ifdef _MSC_VER
# pragma warning(push)
// "This" used in base initializer list
# pragma warning(disable: 4355)
#endif

//-------------------------------------------------------------------------------
Reflex::Internal::CatalogImpl::CatalogImpl():
   fScopes(this), fTypes(this), fName("Global Reflex Catalog"), fCatalog(this) {
//-------------------------------------------------------------------------------
   fScopes.Init();
   fTypes.Init();
}


//-------------------------------------------------------------------------------
Reflex::Internal::CatalogImpl::CatalogImpl(const std::string& name):
   fScopes(this), fTypes(this), fName(name) {
//-------------------------------------------------------------------------------
   fScopes.Init();
   // fundamental types go only into the static dict,
   // and that is initialized via the default constructor:
   // fTypes.Init();
}

#ifdef _MSC_VER
# pragma warning(pop)
#endif

