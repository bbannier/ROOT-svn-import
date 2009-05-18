// @(#)root/reflex:$Name:  $:$Id$
// Author: Axel Naumann, 2009

// Copyright CERN, CH-1211 Geneva 23, 2004-2006, All rights reserved.
//
// Permission to use, copy, modify, and distribute this software for any
// purpose is hereby granted without fee, provided that this copyright and
// permissions notice appear in all copies and derivatives.
//
// This software is provided "as is" without express or implied warranty.

#ifndef Reflex_TypeCatalogImpl
#define Reflex_TypeCatalogImpl

#include "Reflex/internal/TypeName.h"
#include "Reflex/Callback.h"

#include <list>
#include "stl_hash.h"

namespace Reflex {
   namespace Internal {
      class CatalogImpl;

      class TypeCatalogImpl {
      public:

         typedef __gnu_cxx::hash_map<const std::string *, Reflex::TypeName * > Name2TypeNameMap_t;
         typedef __gnu_cxx::hash_map<const char *, Reflex::TypeName * > TypeIdName2TypeNameMap_t;
         typedef std::vector< Reflex::Type > TypeVec_t;

         TypeCatalogImpl(const CatalogImpl* catalog):
            fCatalog(catalog) {}
         ~TypeCatalogImpl() {}

         void Init();

         //const Name2TypeNameMap_t& Name2TypeNameMap() const { return fName2TypeNameMap; }
         //const TypeIdName2TypeNameMap_t& TypeIdName2TypeNameMap() const { return fTypeIdName2TypeNameMap; }
         const TypeVec_t& TypeVec() const { return fTypeVec; }

         Type ByName(const std::string& name) const;
         Type ByTypeInfo(const std::type_info & ti) const;
         void CleanUp() const;

         void Add(TypeName& type, const std::type_info* ti);
         void UpdateTypeId(TypeName& type, const std::type_info & newti,
                           const std::type_info & oldti = typeid(NullType));
         void Remove(TypeName& type);

         // Callbacks
         void UnregisterCallback(const Callback<Type>& cb) {
            fCallbacks.remove(cb);
         }

      private:

         const CatalogImpl*       fCatalog;
         Name2TypeNameMap_t       fName2TypeNameMap;
         TypeIdName2TypeNameMap_t fTypeIdName2TypeNameMap;
         TypeVec_t                fTypeVec;
         std::list<Callback<Type> >  fCallbacks;
      };
   }
}

#endif // Reflex_TypeCatalogImpl
