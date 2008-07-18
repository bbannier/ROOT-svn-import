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
      PairTypeInfoType(const Type& type): fType(type), fTI(type.TypeInfo()) {}
      PairTypeInfoType(const Type& type, const std::type_info& ti): fType(type), fTI(ti) {}
      operator const char* () const { return TypeName(); } // for ContainerAdaptor::Key()
      const char* TypeName() const { return fTI.name(); }
      operator const Type& () const { return fType; }

      void Invalidate() { fType = Type(); }
      bool IsInvalidated() const { return !fType.Id(); }

   private:
      Type fType;
      const std::type_info& fTI;
   };

   //---- Container Adaptor for PairTypeInfoType ----

   template <>
   bool
   ContainerAdaptor::KeyMatches(const char* const & name, const PairTypeInfoType& pti) const {
      return !strcmp(name, pti.TypeName());
   }

   template <>
   bool
   ContainerAdaptor::KeyMatches(const char* const & name, const PairTypeInfoType& pti, const char* &) const {
      return !strcmp(name, pti.TypeName());
   }

   template <>
   void
   ContainerAdaptor::Invalidate(PairTypeInfoType& pti) const {
      pti.Invalidate();
   }

   template <>
   bool
   ContainerAdaptor::IsInvalidated(const PairTypeInfoType& pti) const {
      return pti.IsInvalidated();
   }

   class TypeCatalogImpl {
   public:
      typedef ContainerImpl<std::string, Type> TypeContainer_t;

      TypeCatalogImpl(const CatalogImpl* catalog = 0): fCatalog(catalog) {}
      ~TypeCatalogImpl() {}

      void SetCatalog(const CatalogImpl* catalog) { fCatalog = catalog; };

      const TypeContainer_t& All() const { return fAllTypes; }
      Type ByName(const std::string& name) const;
      Type ByTypeInfo(const std::type_info & ti) const;
      void CleanUp() const;

      static const Type& Get_int() {return Get(kINT);}
      static const Type& Get_float() {return Get(kFLOAT);}
      static const Type& Get(EFUNDAMENTALTYPE);

      void Add(const TypeName& type, const std::type_info * ti);
      void UpdateTypeId(const TypeName& type, const std::type_info & newti,
         const std::type_info & oldti = typeid(NullType));
      void Remove(const TypeName& type);


   private:
      typedef ContainerImpl<const char*, PairTypeInfoType> TypeInfoTypeMap_t;

      const CatalogImpl*    fCatalog;
      TypeContainer_t   fAllTypes;
      TypeInfoTypeMap_t fTypeInfoTypeMap;
   };

   class ScopeCatalogImpl {
   public:
      typedef ContainerImpl<std::string, Scope>            ScopeContainer_t;

      ScopeCatalogImpl(const CatalogImpl* catalog = 0): fCatalog(catalog) {}
      ~ScopeCatalogImpl() {}

      void SetCatalog(const CatalogImpl* catalog) { fCatalog = catalog; };

      const ScopeContainer_t& All() const { return fAllScopes; }
      Scope ByName(const std::string& name) const;
      void CleanUp() const;

      void Add(const ScopeName& scope);
      void Remove(const ScopeName& scope);

      static Scope GlobalScope();

   private:
      const CatalogImpl*    fCatalog;
      ScopeContainer_t  fAllScopes;
   };

   class CatalogImpl {
   public:
      static CatalogImpl& Instance();

      const ScopeCatalogImpl& Scopes() const {return fScopes;}
      const TypeCatalogImpl&  Types() const  {return fTypes;}

   private:
      // allow access to non-const getters, updating scopes and types
      friend class ScopeName;
      friend class TypeName;

      ScopeCatalogImpl& Scopes() {return fScopes;}
      TypeCatalogImpl&  Types()  {return fTypes;}

      CatalogImpl() { fScopes.SetCatalog(this); fTypes.SetCatalog(this); }
      ~CatalogImpl() {}
      ScopeCatalogImpl fScopes;
      TypeCatalogImpl  fTypes;
   };
} // namespace Internal
} // namespace Reflex

#endif
