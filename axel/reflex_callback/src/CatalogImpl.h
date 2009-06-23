// @(#)root/reflex:$Name:  $:$Id$
// Author: Axel Naumann, 2009

// Copyright CERN, CH-1211 Geneva 23, 2004-2006, All rights reserved.
//
// Permission to use, copy, modify, and distribute this software for any
// purpose is hereby granted without fee, provided that this copyright and
// permissions notice appear in all copies and derivatives.
//
// This software is provided "as is" without express or implied warranty.

#ifndef Reflex_CatalogImpl
#define Reflex_CatalogImpl

#include <string>
#include <string.h>
#include <typeinfo>
#include <set>

#include "Reflex/Kernel.h"
#include "Reflex/Type.h"
#include "Reflex/Scope.h"
#include "Reflex/Tools.h"
#include "Reflex/Catalog.h"

#include "ScopeCatalogImpl.h"
#include "TypeCatalogImpl.h"

namespace Reflex {
   class TypeName;
   class ScopeName;

namespace Internal {

   class CatalogImpl {
   public:
      CatalogImpl(const std::string& name);
      ~CatalogImpl();

      static CatalogImpl& Instance();

      const ScopeCatalogImpl& Scopes() const {return fScopes;}
      const TypeCatalogImpl&  Types() const  {return fTypes;}

      ScopeCatalogImpl& Scopes() {return fScopes;}
      TypeCatalogImpl&  Types()  {return fTypes;}

      Catalog ThisCatalog() const { return fCatalog; }

      const std::string& Name() { return fName; }

      // Callbacks
      void RegisterCallback(Callback* cb);
      void UnregisterCallback(Callback* cb);

   private:
      // default constructor for the static instance
      CatalogImpl();

   private:
      ScopeCatalogImpl fScopes;
      TypeCatalogImpl  fTypes;
      std::string      fName;
      Catalog          fCatalog;
      std::set<Callback*> fCallbacks;
   };
}}

#endif // Reflex_CatalogImpl
