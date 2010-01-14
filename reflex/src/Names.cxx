// @(#)root/reflex:$Id$
// Author: Stefan Roiser 2004

// Copyright CERN, CH-1211 Geneva 23, 2004-2006, All rights reserved.
//
// Permission to use, copy, modify, and distribute this software for any
// purpose is hereby granted without fee, provided that this copyright and
// permissions notice appear in all copies and derivatives.
//
// This software is provided "as is" without express or implied warranty.

#ifndef REFLEX_BUILD
#define REFLEX_BUILD
#endif

#include "Reflex/internal/Names.h"
#include "Reflex/Dictionary.h"
#include "Reflex/Type.h"
#include "Reflex/SharedLibraryUtils.h"
#include "Namespace.h"
#include "stl_hash.h"

//-------------------------------------------------------------------------------
typedef __gnu_cxx::hash_map<const std::string *, Reflex::TypeName * > Name2Type_t;
typedef __gnu_cxx::hash_map<const std::string *, Reflex::Scope > Name2Scope_t;
typedef __gnu_cxx::hash_map<const char *, Reflex::TypeName * > TypeId2Type_t;
typedef std::map<std::string, Reflex::Names * > SharedLibrary2Names;
typedef std::vector<Reflex::Names*> NamesVector;

#ifdef _WIN32
#pragma warning( push )
#pragma warning( disable : 4251 )
#endif

/** Private state for Names */
struct RFLX_API Reflex::Names::State {

   /** unique constructor */
   State(const std::string& sharedLibraryName, const NamesVector& bases = NamesVector());

   /** name of the reflected shared library */
   const std::string fSharedLibraryName;

   /** base typenames registry */
   const NamesVector fBases;

   /** map of names to reflected type names. */
   Name2Type_t fTypes;

   /** map of type_info names to reflected type names. */
   TypeId2Type_t fTypeInfos;

   /** map of names to reflected scope names. */
   Name2Scope_t fScopes;

   /** instance of the global scope */
   Namespace* fGlobalScope;

};

#ifdef _WIN32
#pragma warning( pop )
#endif

//-------------------------------------------------------------------------------
static SharedLibrary2Names& AllNames() {
//-------------------------------------------------------------------------------
   static SharedLibrary2Names result;
   return result;
}


//-------------------------------------------------------------------------------
static void RegisterNames(Reflex::Names& names) {
//-------------------------------------------------------------------------------
   AllNames()[names.SharedLibraryName()] = &names;
}


//-------------------------------------------------------------------------------
static void UnregisterNames(const Reflex::Names& names) {
//-------------------------------------------------------------------------------
   AllNames().erase(names.SharedLibraryName());
}


//-------------------------------------------------------------------------------
Reflex::Names& Reflex::Names::Main() {
//-------------------------------------------------------------------------------
   static Names result;
   return result;
}


//-------------------------------------------------------------------------------
Reflex::Names* Reflex::Names::BySharedLibrary(const std::string& sharedLibrary) {
//-------------------------------------------------------------------------------
   const SharedLibrary2Names& allNames = AllNames();
   SharedLibrary2Names::const_iterator i = allNames.find(NamePartOfPath(sharedLibrary));
   if (i == allNames.end())
      return NULL;

   return i->second;
}


//-------------------------------------------------------------------------------
Reflex::Names& Reflex::Names::Defining(const void* functionPointer) {
//-------------------------------------------------------------------------------

   std::string sharedLibrary = SharedLibraryDefining(functionPointer);
   Names* result = Names::BySharedLibrary(sharedLibrary);
   if (!result) {
      // if not found, we must be in non versioning mode, so let's return the Main registry
      return Main();
   }
   return *result;
}


//-------------------------------------------------------------------------------
Reflex::Names& Reflex::Names::FromDictionary(const Reflex::Dictionary& dictionary) {
//-------------------------------------------------------------------------------
   return *dictionary.NamesGet();
}


//-------------------------------------------------------------------------------
Reflex::Names::State::State(const std::string& sharedLibraryName, const NamesVector& bases)
   : fSharedLibraryName(sharedLibraryName)
   , fBases(bases)
   , fGlobalScope(NULL) {
//-------------------------------------------------------------------------------
}


//-------------------------------------------------------------------------------
Reflex::Names::Names()
   : fState(new State("<GLOBAL>")) {
//-------------------------------------------------------------------------------
   RegisterNames(*this);
}


//-------------------------------------------------------------------------------
Reflex::Names::Names(const std::string& sharedLibraryName, const NamesVector& bases)
   : fState(new State(sharedLibraryName, bases)) {
//-------------------------------------------------------------------------------
   RegisterNames(*this);
}


//-------------------------------------------------------------------------------
Reflex::Names::~Names() {
//-------------------------------------------------------------------------------
   //delete fState->fGlobalScope;
   UnregisterNames(*this);
   delete fState;
   fState = NULL;
}


//-------------------------------------------------------------------------------
const std::string& Reflex::Names::SharedLibraryName() const {
//-------------------------------------------------------------------------------
   return fState->fSharedLibraryName;
}


//-------------------------------------------------------------------------------
void Reflex::Names::CollectAllDependencies(std::set<const Names*>& result) const {
//-------------------------------------------------------------------------------
   result.insert(this);
   for(NamesVector::const_iterator i = fState->fBases.begin(); i != fState->fBases.end(); ++i)
   {
      (*i)->CollectAllDependencies(result);
   }
}


//-------------------------------------------------------------------------------
template<typename R, typename C, typename F, typename K>
R FirstTrue(const C& items, F f, const K& k, const R& defaultResult) {
//-------------------------------------------------------------------------------
// searches for the first true result to f, and returns it
   for(typename C::const_iterator i = items.begin(); i != items.end(); ++i)
   {
      R result = f(*i,k);
      if (result)
         return result;
   }
   return defaultResult;
}


//-------------------------------------------------------------------------------
Reflex::TypeName*
Reflex::Names::ByTypeName( const std::string & key ) const {
//-------------------------------------------------------------------------------
// Lookup a type by name.
   Name2Type_t::const_iterator it;
   const Name2Type_t& n2t = fState->fTypes;
   if (key.size()>2 && key[0]==':' && key[1]==':') {
      const std::string & k = key.substr(2);
      it = n2t.find(&k);
   } else {
      it = n2t.find(&key); 
   }
   if( it != n2t.end() )
      return it->second;
   else                  
      return FirstTrue<TypeName*>(fState->fBases, std::mem_fun(&::Reflex::Names::ByTypeName), key, NULL);
}


//-------------------------------------------------------------------------------
void Reflex::Names::RegisterTypeName(const std::string & key, Reflex::TypeName* typeName) {
//-------------------------------------------------------------------------------
   fState->fTypes [ &key ] = typeName;
}


//-------------------------------------------------------------------------------
void Reflex::Names::UnregisterTypeName(const std::string & key ) {
//-------------------------------------------------------------------------------
   fState->fTypes.erase(&key);
}

//-------------------------------------------------------------------------------
Reflex::TypeName*
Reflex::Names::ByTypeInfoGlobal( const std::type_info & ti ) {
//-------------------------------------------------------------------------------
// Lookup a type by type_info.
   Names& names = Names::Defining(&ti);
   return names.ByTypeInfo(ti);
}

//-------------------------------------------------------------------------------
Reflex::TypeName*
Reflex::Names::ByTypeInfo( const std::type_info & ti ) const {
//-------------------------------------------------------------------------------
// Lookup a type by type_info.
   const TypeId2Type_t& id2t = fState->fTypeInfos;
   TypeId2Type_t::const_iterator it = id2t.find(ti.name());
   if( it != id2t.end() )
      return it->second;
   else
      return FirstTrue<TypeName*>(fState->fBases, std::mem_fun(&Names::ByTypeInfo), ti, NULL);
}


//-------------------------------------------------------------------------------
void Reflex::Names::RegisterTypeInfo( const std::type_info & ti, Reflex::TypeName* typeName ) {
//-------------------------------------------------------------------------------
   fState->fTypeInfos [ ti.name() ] = typeName;
}


//-------------------------------------------------------------------------------
void Reflex::Names::UnregisterTypeInfo( const std::type_info & ti ) {
//-------------------------------------------------------------------------------
   fState->fTypeInfos.erase(ti.name());
}

//-------------------------------------------------------------------------------
Reflex::Scope Reflex::Names::GlobalScope() const {
//-------------------------------------------------------------------------------
   // global scope is lazy initialized to avoid a recursive call to Names::Main()
   // at shared library load time.
   if (!fState->fGlobalScope) {
      Names* mthis = const_cast<Names*>(this);
      mthis->fState->fGlobalScope = new Namespace(*mthis);
   }

   return fState->fGlobalScope->ThisScope();
}

//-------------------------------------------------------------------------------
Reflex::Scope
Reflex::Names::ByScopeNameShallow( const std::string & key) const {
//-------------------------------------------------------------------------------
// Lookup a scope by fully qualified name.
   Name2Scope_t::const_iterator it;
   const Name2Scope_t& n2t = fState->fScopes;
   if (key.size()>2 && key[0]==':' && key[1]==':') {
      const std::string & k = key.substr(2);
      it = n2t.find(&k);
   } else {
      it = n2t.find(&key);
   }
   if (it != n2t.end() ) return it->second;
   else {
      // HERE STARTS AN UGLY HACK WHICH HAS TO BE UNDONE ASAP
      // (also remove inlcude Reflex/Type.h)
      // Couldn't this be moved at the top level Scope::ByName... functions ?
      Type t = Type::ByName(key, *const_cast<Names*>(this) );
      if ( t && t.IsTypedef()) {
         while ( t.IsTypedef()) t = t.ToType();
         if ( t.IsClass() || t.IsEnum() || t.IsUnion() ) return ByScopeNameShallow(t.Name());
      }
      // END OF UGLY HACK
      return Dummy::Scope();
   }
}



//-------------------------------------------------------------------------------
Reflex::Scope
Reflex::Names::ByScopeName( const std::string & key ) const {
//-------------------------------------------------------------------------------
// Lookup a scope by fully qualified name.
   Reflex::Scope result = ByScopeNameShallow(key);
   if (result)
      return result;
   
   return FirstTrue<Scope>(fState->fBases, std::mem_fun(&::Reflex::Names::ByScopeName), key, Dummy::Scope());
}


//-------------------------------------------------------------------------------
void Reflex::Names::RegisterScopeName(const std::string & key, const Reflex::Scope& scope) {
//-------------------------------------------------------------------------------
   fState->fScopes [ &key ] = scope;
}


//-------------------------------------------------------------------------------
void Reflex::Names::UnregisterScopeName(const std::string & key ) {
//-------------------------------------------------------------------------------
   fState->fScopes.erase(&key);
}
