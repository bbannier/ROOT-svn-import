// @(#)root/reflex:$Name:  $:$Id$
// Author: Axel Naumann, 2007

// Copyright CERN, CH-1211 Geneva 23, 2004-2006, All rights reserved.
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
#include "Reflex/Container.h"
#include "Reflex/Tools.h"

namespace Reflex {

   class Catalog;

   namespace Internal {
      class TypeName;
      class ScopeName;

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
         Reflex::Type fType;
         const std::type_info& fTI;
      };

   }

   //---- Container Adaptor for PairTypeInfoType ----

   template <>
   bool
   ContainerAdaptor::KeyMatches(const char* const & name, const Internal::PairTypeInfoType& pti) const {
      return !strcmp(name, pti.TypeName());
   }

   template <>
   bool
   ContainerAdaptor::KeyMatches(const char* const & name, const Internal::PairTypeInfoType& pti, const char* &) const {
      return !strcmp(name, pti.TypeName());
   }

   template <>
   void
   ContainerAdaptor::Invalidate(Internal::PairTypeInfoType& pti) const {
      pti.Invalidate();
   }

   template <>
   bool
   ContainerAdaptor::IsInvalidated(const Internal::PairTypeInfoType& pti) const {
      return pti.IsInvalidated();
   }

   //---- Container Adaptor for Type ----

   template <>
   std::string
   ContainerAdaptor::Key(const Type& type) const {
      return type.Name();
   }

   template <>
   const std::string&
   ContainerAdaptor::Key(const Type& type, std::string& buf) const {
      return type.Name(buf);
   }

   //---- Container Adaptor for Scope ----

   template <>
   std::string
   ContainerAdaptor::Key(const Scope& scope) const {
      return scope.Name();
   }

   template <>
   const std::string&
   ContainerAdaptor::Key(const Scope& scope, std::string& buf) const {
      return scope.Name(buf);
   }



   class TypeCatalog {
   public:
      typedef Container<std::string, Type> TypeContainer_t;

      TypeCatalog(const Catalog* catalog = 0): fCatalog(catalog) {}
      ~TypeCatalog() {}

      void SetCatalog(const Catalog* catalog) { fCatalog = catalog; };

      const TypeContainer_t& All() const { return fAllTypes; }
      Type ByName(const std::string& name) const;
      Type ByTypeInfo(const std::type_info & ti) const;
      void CleanUp() const;

      static const Type& Get_int() {return Get(kINT);}
      static const Type& Get_float() {return Get(kFLOAT);}
      static const Type& Get(EFUNDAMENTALTYPE);

      void Add(const Internal::TypeName& type, const std::type_info * ti);
      void UpdateTypeId(const Internal::TypeName& type, const std::type_info & newti,
         const std::type_info & oldti = typeid(NullType));
      void Remove(const Internal::TypeName& type);


   private:
      typedef Container<const char*, Internal::PairTypeInfoType> TypeInfoTypeMap_t;

      const Catalog*    fCatalog;
      TypeContainer_t   fAllTypes;
      TypeInfoTypeMap_t fTypeInfoTypeMap;
   };

   class ScopeCatalog {
   public:
      typedef Container<std::string, Scope>            ScopeContainer_t;

      ScopeCatalog(const Catalog* catalog = 0): fCatalog(catalog) {}
      ~ScopeCatalog() {}

      void SetCatalog(const Catalog* catalog) { fCatalog = catalog; };

      const ScopeContainer_t& All() const { return fAllScopes; }
      Scope ByName(const std::string& name) const;
      void CleanUp() const;

      void Add(const Internal::ScopeName& scope);
      void Remove(const Internal::ScopeName& scope);

      static Scope GlobalScope();

   private:
      const Catalog*    fCatalog;
      ScopeContainer_t  fAllScopes;
   };

   class Catalog {
   public:
      static Catalog& Instance();

      const ScopeCatalog& Scopes() const {return fScopes;}
      const TypeCatalog&  Types() const  {return fTypes;}

   private:
      // allow access to non-const getters, updating scopes and types
      friend class Internal::ScopeName;
      friend class Internal::TypeName;

      ScopeCatalog& Scopes() {return fScopes;}
      TypeCatalog&  Types()  {return fTypes;}

      Catalog() { fScopes.SetCatalog(this); fTypes.SetCatalog(this); }
      ~Catalog() {}
      ScopeCatalog fScopes;
      TypeCatalog  fTypes;
   };

}

#endif
