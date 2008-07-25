// @(#)root/reflex:$Name:  $:$Id$
// Author: Axel Naumann, 2007

// Copyright CERN, CH-1211 Geneva 23, 2004-2007, All rights reserved.
//
// Permission to use, copy, modify, and distribute this software for any
// purpose is hereby granted without fee, provided that this copyright and
// permissions notice appear in all copies and derivatives.
//
// This software is provided "as is" without express or implied warranty.

#ifndef REFLEX_BUILD
#define REFLEX_BUILD
#endif

#include "CatalogImpl.h"

#include "ScopeName.h"
#include "Reflex/EntityProperty.h"
#include "Namespace.h"

//-------------------------------------------------------------------------------
Reflex::Internal::CatalogImpl&
Reflex::Internal::CatalogImpl::Instance() {
//-------------------------------------------------------------------------------
// Return the global instance of the reflection catalog.
   static CatalogImpl instance;

   return instance;
}

//-------------------------------------------------------------------------------
Reflex::Type
Reflex::Internal::TypeCatalogImpl::ByName( const std::string & name ) const {
//-------------------------------------------------------------------------------
// Lookup a type by name.
   size_t pos = 0;
   while (name[pos] == ':') ++pos;
   TypeContainer_t::const_iterator it = fAllTypes.Find(name.substr(pos));
   if (it != fAllTypes.End()) return *it;
   return Dummy::Type();
}


//-------------------------------------------------------------------------------
Reflex::Type
Reflex::Internal::TypeCatalogImpl::ByTypeInfo( const std::type_info & ti ) const {
//-------------------------------------------------------------------------------
// Lookup a type by type_info.
   TypeInfoTypeMap_t::const_iterator it = fTypeInfoTypeMap.Find(ti.name());
   if (it != fTypeInfoTypeMap.End()) return *it;
   return Dummy::Type();
}


//-------------------------------------------------------------------------------
void Reflex::Internal::TypeCatalogImpl::CleanUp() const {
//-------------------------------------------------------------------------------
   // Cleanup memory allocations for types.
   /* SHOULD BE DONE BY ScopeName!
   for ( TypeContainer_t::const_iterator it = fAllTypes.Begin(); it != fAllTypes.End(); ++it ) {
      TypeName * tn = (TypeName*)it->Id();
      Type * t = tn->fThisType;
      if ( *t ) t->Unload();
   }
   */
}


//-------------------------------------------------------------------------------
void Reflex::Internal::TypeCatalogImpl::Add(const Reflex::Internal::TypeName& type, const std::type_info * ti) {
//-------------------------------------------------------------------------------
// Add a type_info to the map.
   Type t(&type, 0);
   fAllTypes.Insert(t);
   if (ti) fTypeInfoTypeMap.Insert(Internal::PairTypeInfoType(t, *ti));
}

//-------------------------------------------------------------------------------
void Reflex::Internal::TypeCatalogImpl::UpdateTypeId(const Reflex::Internal::TypeName& type, const std::type_info & newti, 
                                    const std::type_info & oldti /* =typeid(NullType) */) {
//-------------------------------------------------------------------------------
// Update a type_info in the map.
   Type t(&type, 0);
   if (oldti != typeid(NullType))
      fTypeInfoTypeMap.Remove(Internal::PairTypeInfoType(t, oldti));
   if (newti != typeid(NullType))
      fTypeInfoTypeMap.Insert(Internal::PairTypeInfoType(t, newti));
}


//-------------------------------------------------------------------------------
void Reflex::Internal::ScopeCatalogImpl::Add(const Reflex::Internal::ScopeName& scope) {
//-------------------------------------------------------------------------------
// Add a scope to the map.
   Scope s(&scope);
   fAllScopes.Insert(s);
}


//-------------------------------------------------------------------------------
Reflex::Scope
Reflex::Internal::ScopeCatalogImpl::ByName( const std::string & name ) const {
//-------------------------------------------------------------------------------
// Lookup a scope by fully qualified name.
   size_t pos = 0;
   while (name[pos] == ':') ++pos;
   const std::string & k = name.substr(pos);
   ScopeContainer_t::const_iterator it = fAllScopes.Find(k);
   if (it != fAllScopes.End() ) return *it;
   //else                        return Dummy::Scope();
   // HERE STARTS AN UGLY HACK WHICH HAS TO BE UNDONE ASAP
   // (also remove inlcude Reflex/Type.h)
   Type t = fCatalog->Types().ByName(name);
   if ( t && t.Is(gTypedef)) {
      while ( t.Is(gTypedef)) t = t.ToType();
      if ( t.IsClass() || t.Is(gEnum || gUnion) ) return t.operator Scope ();
   }
   return Dummy::Scope();
   // END OF UGLY HACK
}


//-------------------------------------------------------------------------------
void Reflex::Internal::ScopeCatalogImpl::CleanUp() const {
//-------------------------------------------------------------------------------
   // Cleanup memory allocations for scopes.
   /* SHOULD BE DONE BY ScopeName!
   ScopeVec_t::iterator it;
   for ( it = fAllScopes.Begin(); it != fAllScopes.End(); ++it ) {
      Scope * s = ((ScopeName*)it->Id())->fThisScope;
      if ( *s ) s->Unload();
   }*/
}

//-------------------------------------------------------------------------------
Reflex::Scope Reflex::Internal::ScopeCatalogImpl::GlobalScope() {
//-------------------------------------------------------------------------------
// Return the global scope's Scope object.
   return Internal::Namespace::GlobalScope();
}
