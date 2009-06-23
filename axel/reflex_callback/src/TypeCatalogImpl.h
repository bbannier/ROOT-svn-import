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

#include <set>
#include <vector>
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

         const TypeVec_t& TypeVec() const { return fTypeVec; }

         Type ByName(const std::string& name) const;
         TypeName* ByTypeName(const std::string& name) const;
         Type ByTypeInfo(const std::type_info & ti) const;
         void CleanUp() const;

         void Add(TypeName& type, const std::type_info* ti);
         void UpdateTypeId(TypeName& type, const std::type_info & newti,
                           const std::type_info & oldti = typeid(NullType));
         void Remove(TypeName& type);

         // Callbacks
         void RegisterCallback(const Callback& cb);
         void UnregisterCallback(const Callback& cb);

      private:
         void NofifyAnonymous(NotifyInfo& ni);

         const CatalogImpl*       fCatalog;
         Name2TypeNameMap_t       fName2TypeNameMap;
         TypeIdName2TypeNameMap_t fTypeIdName2TypeNameMap;
         TypeVec_t                fTypeVec;
         std::set<Callback*>      fAnonymousCallbacks; // unnamed callbacks
         std::map<std::string, std::set<Callback*> > fOrphanedCallbacks; // named callbacks for unknown type
      };
   }
}

//-------------------------------------------------------------------------------
inline Reflex::Type
Reflex::Internal::TypeCatalogImpl::ByName(const std::string & name) const {
//-------------------------------------------------------------------------------
// Lookup a type by name.
   TypeName* tn = ByTypeName(name);
   if (tn) return tn->ThisType();
   return Dummy::Type();
}


#endif // Reflex_TypeCatalogImpl
