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

#include "Reflex/internal/ScopeName.h"

#include "Reflex/Scope.h"
#include "Reflex/Type.h"
#include "Reflex/Tools.h"
#include "Reflex/internal/ScopeBase.h"
#include "Reflex/internal/OwnedMember.h"
#include "CatalogImpl.h"
#include "stl_hash.h"
#include <vector>

/*
//-------------------------------------------------------------------------------
typedef __gnu_cxx::hash_map < const std::string *, Reflex::Scope > Name2Scope_t;
typedef std::vector< Reflex::Scope > ScopeVec_t;

//-------------------------------------------------------------------------------
static Name2Scope_t & sScopes() {
//-------------------------------------------------------------------------------
// Static wrapper around scope map.
   static Name2Scope_t* m = 0;
   if (!m) m = new Name2Scope_t;
   return *m;
}


//-------------------------------------------------------------------------------
static ScopeVec_t & sScopeVec() {
//-------------------------------------------------------------------------------
// Static wrapper around scope vector.
   static ScopeVec_t* m = 0;
   if (!m) m = new ScopeVec_t;
   return *m;
}
*/

//-------------------------------------------------------------------------------
Reflex::ScopeName::ScopeName( const char * name,
                              ScopeBase * scopeBase,
                              const Catalog& catalog) 
   : fName(name),
     fScopeBase(scopeBase),
     fCatalog(catalog) {
//-------------------------------------------------------------------------------
// Create the scope name dictionary info.
   fThisScope = new Scope(this);
   catalog.ToImpl()->Scopes().Add(*this);
   //---Build recursively the declaring scopeNames
   if( fName != "@N@I@R@V@A@N@A@" ) {
      std::string decl_name = Tools::GetScopeName(fName);
      if ( ! catalog.ScopeByName( decl_name ).Id() )
         new ScopeName( decl_name.c_str(), 0, catalog );
   }
}


//-------------------------------------------------------------------------------
Reflex::ScopeName::~ScopeName() {
//-------------------------------------------------------------------------------
// Destructor.
   if (fScopeBase) {
      fScopeBase->DeclaringScope(Scope());
      delete fScopeBase;
   }
   delete fThisScope;
}


//-------------------------------------------------------------------------------
Reflex::Scope Reflex::ScopeName::ByName( const std::string & name ) {
//-------------------------------------------------------------------------------
// Lookup a scope by fully qualified name.
   return Catalog::Instance().ScopeByName(name);
}


//-------------------------------------------------------------------------------
void Reflex::ScopeName::HideName() {
//-------------------------------------------------------------------------------
// Append the string " @HIDDEN@" to a scope name.
   if ( fName.length() == 0 || fName[fName.length()-1] != '@' ) {
      fCatalog.ToImpl()->Scopes().Remove(*this);
      fName += " @HIDDEN@";
      fCatalog.ToImpl()->Scopes().Add(*this);
   }
}

//-------------------------------------------------------------------------------
void Reflex::ScopeName::UnhideName() {
   //-------------------------------------------------------------------------------
   // Remove the string " @HIDDEN@" to a scope name.
   static const unsigned int len = strlen(" @HIDDEN@");
   if ( fName.length() > len
        && fName[fName.length()-1] == '@'
        && 0==strcmp(" @HIDDEN@",fName.c_str()+fName.length()-len) ){
      fCatalog.ToImpl()->Scopes().Remove(*this);
      fName.erase(fName.length()-len);
      fCatalog.ToImpl()->Scopes().Add(*this);
   }
}

//-------------------------------------------------------------------------------
Reflex::Scope Reflex::ScopeName::ThisScope() const {
//-------------------------------------------------------------------------------
// Return the scope corresponding to this scope.
   return *fThisScope;
}


//-------------------------------------------------------------------------------
Reflex::Scope Reflex::ScopeName::ScopeAt( size_t nth ) {
//-------------------------------------------------------------------------------
// Return the nth scope defined in Reflex.
   return Catalog::Instance().ScopeAt(nth);
}


//-------------------------------------------------------------------------------
size_t Reflex::ScopeName::ScopeSize() {
//-------------------------------------------------------------------------------
// Return the number of scopes defined in Reflex.
   return Catalog::Instance().ScopeSize();
}


//-------------------------------------------------------------------------------
Reflex::Scope_Iterator Reflex::ScopeName::Scope_Begin() {
//-------------------------------------------------------------------------------
// Return the begin iterator of the scope collection.
   return Catalog::Instance().Scope_Begin();
}


//-------------------------------------------------------------------------------
Reflex::Scope_Iterator Reflex::ScopeName::Scope_End() {
//-------------------------------------------------------------------------------
// Return the end iterator of the scope collection.
   return Catalog::Instance().Scope_End();
}


//-------------------------------------------------------------------------------
Reflex::Reverse_Scope_Iterator Reflex::ScopeName::Scope_RBegin() {
//-------------------------------------------------------------------------------
// Return the rbegin iterator of the scope collection.
   return Catalog::Instance().Scope_RBegin();
}


//-------------------------------------------------------------------------------
Reflex::Reverse_Scope_Iterator Reflex::ScopeName::Scope_REnd() {
//-------------------------------------------------------------------------------
// Return the rend iterator of the scope collection.
   return Catalog::Instance().Scope_REnd();
}


//-------------------------------------------------------------------------------
void
Reflex::ScopeName::RegisterCallback(Callback& cb) {
//-------------------------------------------------------------------------------
// Register a callback for this type
   if (!fCallbacks) fCallbacks = new std::set<Callback*>;
   fCallbacks->insert(&cb);
}


//-------------------------------------------------------------------------------
void
Reflex::ScopeName::UnregisterCallback(Callback& cb) {
//-------------------------------------------------------------------------------
// Remove a callback for this type
   if (fCallbacks) {
      std::set<Callback*>::iterator i = fCallbacks->find(&cb);
      if (i != fCallbacks->end())
         fCallbacks->erase(i);
   }
}

