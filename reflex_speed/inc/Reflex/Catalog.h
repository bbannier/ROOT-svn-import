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
      class TypeImpl;
      class ScopeImpl;
   }

   class TypeCatalog {
   public:
      typedef Container<Type> TypeContainer_t;

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

      void Add(const Internal::TypeImpl& type, const std::type_info * ti);
      void UpdateTypeId(const Internal::TypeImpl& type, const std::type_info & newti,
         const std::type_info & oldti = typeid(NullType));
      void Remove(const Internal::TypeImpl& type);


   private:
      class PairTypeInfoType {
      public:
         PairTypeInfoType(const Type& type): fType(type), fTI(type.TypeId()) {}
         PairTypeInfoType(const Type& type, const std::type_info& ti): fType(type), fTI(ti) {}
         const char* Name() const { return fType.TypeId().name(); }
         const char* Name(std::string&) const { return Name(); }
         operator const Type& () const { return fType; }
         bool operator!=(const PairTypeInfoType& rhs) const { return fType != rhs.fType; }

      private:
         Reflex::Type fType;
         const std::type_info& fTI;
      };

      typedef Container<PairTypeInfoType> TypeInfoTypeMap_t;

      const Catalog*    fCatalog;
      TypeContainer_t   fAllTypes;
      TypeInfoTypeMap_t fTypeInfoTypeMap;
   };

   class ScopeCatalog {
   public:
      typedef Container<Scope>            ScopeContainer_t;
      typedef Container<Member>           MemberContainer_t;

      ScopeCatalog(const Catalog* catalog = 0): fCatalog(catalog) {}
      ~ScopeCatalog() {}

      void SetCatalog(const Catalog* catalog) { fCatalog = catalog; };

      const ScopeContainer_t& All() const { return fAllScopes; }
      Scope ByName(const std::string& name) const;
      void CleanUp() const;

      void Add(const Internal::ScopeImpl& scope);
      void Remove(const Internal::ScopeImpl& scope);

      static const Scope& GlobalScope();

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
      friend class Internal::ScopeImpl;
      friend class Internal::TypeImpl;

      ScopeCatalog& Scopes() {return fScopes;}
      TypeCatalog&  Types()  {return fTypes;}

      Catalog() { fScopes.SetCatalog(this); fTypes.SetCatalog(this); }
      ~Catalog() {}
      ScopeCatalog fScopes;
      TypeCatalog  fTypes;
   };

}

#endif
