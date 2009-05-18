// @(#)root/reflex:$Name:  $:$Id$
// Author: Axel Naumann, 2009

// Copyright CERN, CH-1211 Geneva 23, 2004-2006, All rights reserved.
//
// Permission to use, copy, modify, and distribute this software for any
// purpose is hereby granted without fee, provided that this copyright and
// permissions notice appear in all copies and derivatives.
//
// This software is provided "as is" without express or implied warranty.

#ifndef Reflex_ScopeCatalogImpl
#define Reflex_ScopeCatalogImpl

#include "Reflex/Scope.h"

#include "stl_hash.h"

namespace Reflex {
   namespace Internal {

      class CatalogImpl;

      class ScopeCatalogImpl {
      public:
         typedef __gnu_cxx::hash_map<const std::string*, Reflex::Scope> Name2ScopeMap_t;
         typedef std::vector<Reflex::Scope> ScopeVec_t;

         ScopeCatalogImpl(const CatalogImpl* catalog):
            fCatalog(catalog), fGlobalScope(0), fNirvana(0) {}
         ~ScopeCatalogImpl();

         void Init();

         void SetCatalog(const CatalogImpl* catalog) { fCatalog = catalog; };

         const Name2ScopeMap_t& Name2ScopeMap() const { return fName2ScopeMap; }
         const ScopeVec_t& ScopeVec() const { return fScopeVec; }
         Scope ByName(const std::string& name) const;

         void Add(ScopeName& scope);
         void Remove(ScopeName& scope);

         ScopeName* GlobalScope() const { return fGlobalScope; }
         ScopeName* __NIRVANA__() const { return fNirvana; }

         // Callbacks
         void UnregisterCallback(const CallbackBase& cb) const {
            fCallbacks.remove(cb);
         }

      private:
         const CatalogImpl*   fCatalog;
         Name2ScopeMap_t      fName2ScopeMap;
         ScopeVec_t           fScopeVec;
         ScopeName*           fGlobalScope;
         ScopeName*           fNirvana;
         std::list<CallbackBase*>  fCallbacks;
      };
   }
}

#endif // Reflex_ScopeCatalogImpl
