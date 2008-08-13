// @(#)root/reflex:$Name:  $:$Id$
// Author: Axel Naumann, 2007

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
#include "Reflex/Kernel.h"
#include "Reflex/Type.h"
#include "Reflex/Scope.h"
#include "Reflex/Tools.h"
#include "Reflex/Catalog.h"

#include "ContainerImpl.h"

namespace Reflex {
namespace Internal {

   class TypeName;
   class ScopeName;
   class CatalogImpl;

   class PairTypeInfoType {
   public:
      PairTypeInfoType(): fType(), fTI(typeid(UnknownType)) {}
      PairTypeInfoType(const TypeName& type);
      PairTypeInfoType(const TypeName& type, const std::type_info& ti): fType(&type), fTI(ti) {}
      operator const char* () const { return Name(); } // for ContainerTraits::Key()
      const char* Name() const { return fTI.name(); }
      operator const Type () const { return fType ? fType->ThisType() : Type(); }

      void Invalidate() { fType = 0; }
      bool IsInvalidated() const { return !fType; }

   private:
      const TypeName* fType;
      const std::type_info& fTI;
   };

   //---- Container Traits for PairTypeInfoType ----

   template <>
   inline bool
   ContainerTraits::KeyMatches(const char* const & name, const PairTypeInfoType& pti) const {
      return !strcmp(name, pti.Name());
   }

   template <>
   inline bool
   ContainerTraits::KeyMatches(const char* const & name, const PairTypeInfoType& pti, const char* &) const {
      return !strcmp(name, pti.Name());
   }

   template <>
   inline void
   ContainerTraits::Invalidate(PairTypeInfoType& pti) const {
      pti.Invalidate();
   }

   template <>
   inline bool
   ContainerTraits::IsInvalidated(const PairTypeInfoType& pti) const {
      return pti.IsInvalidated();
   }

   class TypeCatalogImpl {
   public:
      typedef ContainerImpl<std::string, TypeName*, kUnique> TypeContainer_t;

      TypeCatalogImpl(const CatalogImpl* catalog);
      ~TypeCatalogImpl() {}

      void SetCatalog(const CatalogImpl* catalog) { fCatalog = catalog; };

      const TypeContainer_t& All() const { return fAllTypes; }
      Type ByName(const std::string& name) const;
      Type ByTypeInfo(const std::type_info & ti) const;
      void CleanUp() const;

      static const Type& Get(EFUNDAMENTALTYPE);

      void Add(TypeName& type, const std::type_info * ti);
      void UpdateTypeId(const TypeName& type, const std::type_info & newti,
         const std::type_info & oldti = typeid(NullType));
      void Remove(TypeName& type);


   private:
      typedef ContainerImpl<const char*, PairTypeInfoType, kUnique> TypeInfoTypeMap_t;

      const CatalogImpl* fCatalog;
      TypeContainer_t    fAllTypes;
      TypeInfoTypeMap_t  fTypeInfoTypeMap;
   };

   class ScopeCatalogImpl {
   public:
      typedef ContainerImpl<std::string, ScopeName*>            ScopeContainer_t;

      ScopeCatalogImpl(const CatalogImpl* catalog = 0);
      ~ScopeCatalogImpl() {}

      void SetCatalog(const CatalogImpl* catalog) { fCatalog = catalog; };

      const ScopeContainer_t& All() const { return fAllScopes; }
      Scope ByName(const std::string& name) const;
      void CleanUp() const;

      void Add(ScopeName& scope);
      void Remove(ScopeName& scope);

      Scope GlobalScope() const;

   private:
      const CatalogImpl* fCatalog;
      ScopeContainer_t   fAllScopes;
      ScopeName*         fNirvana;
   };

   class CatalogImpl {
   public:
      CatalogImpl(): fScopes(this), fTypes(this) {}
      ~CatalogImpl() {}

      static CatalogImpl& Instance();

      const ScopeCatalogImpl& Scopes() const {return fScopes;}
      const TypeCatalogImpl&  Types() const  {return fTypes;}

      ScopeCatalogImpl& Scopes() {return fScopes;}
      TypeCatalogImpl&  Types()  {return fTypes;}

      const Catalog& ThisCatalog() const { return fThisCatalog; }

   private:
      ScopeCatalogImpl fScopes;
      TypeCatalogImpl  fTypes;
      Catalog          fThisCatalog;
   };
} // namespace Internal
} // namespace Reflex

#endif
