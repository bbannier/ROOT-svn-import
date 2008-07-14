// @(#)root/reflex:$Name:  $:$Id$
// Author: Axel Naumann, 2008

// Copyright CERN, CH-1211 Geneva 23, 2004-2008, All rights reserved.
//
// Permission to use, copy, modify, and distribute this software for any
// purpose is hereby granted without fee, provided that this copyright and
// permissions notice appear in all copies and derivatives.
//
// This software is provided "as is" without express or implied warranty.

#ifndef Reflex_Catalog
#define Reflex_Catalog

#include <string>
#include <typeinfo>
#include "Reflex/Kernel.h"
#include "Reflex/Type.h"
#include "Reflex/Scope.h"
#include "Reflex/Tools.h"

namespace Reflex {

   namespace Internal {
      class CatalogImpl;
   }

   template <typename T> class Collection;

   class RFLX_API Catalog {
   public:
      Catalog(const Internal::CatalogImpl* catalog = 0): fImpl(catalog) {}
      ~Catalog() {}

      Type ByName(const std::string& name) const;
      Type ByTypeInfo(const std::type_info & ti) const;
      Scope ScopeByName(const std::string& name) const;

      // iterator access
      const Collection<Type>&  Types() const;
      const Collection<Scope>& Scopes() const;

      // shortcut access:
      static Scope GlobalScope();
      static Type Get_int() {return Get(kINT);}
      static Type Get_float() {return Get(kFLOAT);}
      static Type Get(EFUNDAMENTALTYPE);


   private:
      const Internal::CatalogImpl* fImpl;
   };

} // namespace Reflex

#endif
