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
      PairTypeInfoType(): fType(0), fTI(typeid(UnknownType)) {}
      PairTypeInfoType(const TypeName& type);
      PairTypeInfoType(const TypeName& type, const std::type_info& ti): fType(&type), fTI(ti) {}
      operator const char* () const { return Name(); } // for ContainerTraits::Key()
      const char* Name() const { return fTI.name(); }
      const char* Name(const char*&) const { return fTI.name(); }
      operator const Type () const { return fType ? fType->ThisType() : Type(); }

      void Invalidate() { fType = 0; }
      bool IsValid() const { return fType; }

   private:
      const TypeName* fType;
      const std::type_info& fTI;
   };

   //---- Container Traits for PairTypeInfoType ----

   template <>
   struct NodeValidator<PairTypeInfoType> {
      // set a value to invalid (e.g. for iterators pointing to removed nodes)
      static void Invalidate(PairTypeInfoType& value) { value.Invalidate(); }
      // check whether a value is invalidated (e.g. for iterators pointing to removed nodes)
      static bool IsValid(const PairTypeInfoType& value) { return value.IsValid(); }
   };

   // Specialization of key extraction for VALUE=TypeName*
   template <>
   struct ContainerTraits_KeyExtractor<std::string, Reflex::Internal::TypeName*> {
      const std::string& Get(const Reflex::Internal::TypeName* v) { return v->Name(); }
      const std::string& Get(const Reflex::Internal::TypeName* v, std::string&) { return v->Name(); }
   };

   class TypeCatalogImpl {
   public:
      typedef ContainerImpl<std::string, TypeName*, kUnique> TypeNameContainer_t;
      typedef ContainerImpl<std::string, Type, kMany> TypeContainer_t;

      TypeCatalogImpl(const CatalogImpl* catalog): fCatalog(catalog) {}
      ~TypeCatalogImpl() {}

      void Init();

      const TypeNameContainer_t& AllNames() const { return fAllTypeNames; }
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
