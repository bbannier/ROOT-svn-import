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

   class TypeInfoType: public ::Reflex::Type {
   public:
      TypeInfoType(): fTypeinfo(0) {}
      TypeInfoType(const TypeName& tn):
         Type(&tn), fTypeinfo(0) {}
      TypeInfoType(const TypeName& tn, const std::type_info& ti):
         Type(&tn), fTypeinfo(&ti) {}
      const char* Name() const { return fTypeinfo ? fTypeinfo->name() : "[UNRESOLVED]"; }
      const char* Name(const char*&) const { return Name(); }
      void Invalidate() { fTypeinfo = 0; }
      bool IsValid() const { return fTypeinfo; }
      operator const char*() const { return Name(); }

   private:
      const std::type_info* fTypeinfo; // typeinfo reference for the type
   };

   template <>
   struct NodeValidator<TypeInfoType> {
      // set a value to invalid (e.g. for iterators pointing to removed nodes)
      static void Invalidate(TypeInfoType& value) { value.Invalidate(); }
      // check whether a value is invalidated (e.g. for iterators pointing to removed nodes)
      static bool IsValid(const TypeInfoType& value) { return value.IsValid(); }
   };

   // Specialization of key extraction for VALUE=TypeName*
   template <>
   struct ContainerTraits_KeyExtractor<std::string, Reflex::Internal::TypeName*> {
      const std::string& Get(const Reflex::Internal::TypeName* v) { return v->Name(); }
      const std::string& Get(const Reflex::Internal::TypeName* v, std::string&) { return v->Name(); }
   };

   class TypeCatalogImpl {
   public:
      typedef ContainerImpl<std::string, TypeName*, kUnique>         TypeNameContainer_t;
      typedef ContainerImpl<std::string, Type, kMany>                TypeContainer_t;
      typedef ContainerImpl<const char*, TypeInfoType, kUnique>  TypeInfoTypeMap_t;

      TypeCatalogImpl(const CatalogImpl* catalog):
      fCatalog(catalog), fAllTypes(&fTypeInfoTypeMap), fTypeInfoTypeMap(&fAllTypes) {}
      ~TypeCatalogImpl() {}

      void Init();

      const TypeNameContainer_t& AllNames() const { return fAllTypeNames; }
      const TypeContainer_t& All() const { return fAllTypes; }
      Type ByName(const std::string& name) const;
      Type ByTypeInfo(const std::type_info & ti) const;
      void CleanUp() const;

      static const Type& Get(EFUNDAMENTALTYPE);

      void Add(TypeName& type, const std::type_info* ti);
      void UpdateTypeId(const TypeName& type, const std::type_info & newti,
         const std::type_info & oldti = typeid(NullType));
      void Remove(TypeName& type);


   private:

      const CatalogImpl*  fCatalog;
      TypeNameContainer_t fAllTypeNames;
      TypeContainer_t     fAllTypes;
      TypeInfoTypeMap_t   fTypeInfoTypeMap;
   };

   class ScopeCatalogImpl {
   public:
      typedef ContainerImpl<std::string, ScopeName*> ScopeNameContainer_t;
      typedef ContainerImpl<std::string, Scope> ScopeContainer_t;

      ScopeCatalogImpl(const CatalogImpl* catalog):
         fCatalog(catalog), fGlobalScope(0), fNirvana(0) {}
      ~ScopeCatalogImpl() {}

      void Init();

      void SetCatalog(const CatalogImpl* catalog) { fCatalog = catalog; };

      const ScopeNameContainer_t& AllNames() const { return fAllScopeNames; }
      const ScopeContainer_t& All() const { return fAllScopes; }
      Scope ByName(const std::string& name) const;
      void CleanUp() const;

      void Add(ScopeName& scope);
      void Remove(ScopeName& scope);

      Scope GlobalScope() const;
      Scope __NIRVANA__() const;

   private:
      const CatalogImpl*   fCatalog;
      ScopeNameContainer_t fAllScopeNames;
      ScopeContainer_t     fAllScopes;
      ScopeName*           fGlobalScope;
      ScopeName*           fNirvana;
   };

   class CatalogImpl {
   public:
      CatalogImpl(const std::string& name);
      ~CatalogImpl() {}

      static CatalogImpl& Instance();

      const ScopeCatalogImpl& Scopes() const {return fScopes;}
      const TypeCatalogImpl&  Types() const  {return fTypes;}

      ScopeCatalogImpl& Scopes() {return fScopes;}
      TypeCatalogImpl&  Types()  {return fTypes;}

      Catalog ThisCatalog() const { return const_cast<CatalogImpl*>(this); }

      const std::string& Name() { return fName; }

   private:
      // default constructor for the static instance
      CatalogImpl();

   private:
      ScopeCatalogImpl fScopes;
      TypeCatalogImpl  fTypes;
      std::string      fName;
   };
} // namespace Internal
} // namespace Reflex

#endif
