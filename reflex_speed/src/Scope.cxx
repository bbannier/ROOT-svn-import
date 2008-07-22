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

#include "Reflex/Scope.h"

#include "Reflex/internal/ScopeBase.h"
#include "Reflex/Member.h"
#include "Reflex/Type.h"
#include "Reflex/TypeTemplate.h"
#include "Reflex/MemberTemplate.h"
#include "Reflex/Base.h"

#include "Reflex/Tools.h"
#include "Class.h"

//-------------------------------------------------------------------------------
Reflex::Scope::operator bool () const {
//-------------------------------------------------------------------------------
   return ( this->fScopeName && this->fScopeName->fScopeBase );
}


//-------------------------------------------------------------------------------
Reflex::Scope & Reflex::Scope::__NIRVANA__() {
//-------------------------------------------------------------------------------
// static wraper around NIRVANA, the base of the top scope.
   static Scope s = Scope( new Internal::ScopeName( "@N@I@R@V@A@N@A@", 0 ));
   return s;
}


//-------------------------------------------------------------------------------
Reflex::Scope::operator Reflex::Type () const {
//-------------------------------------------------------------------------------
// Conversion operator to Type. If this scope is not a Type, returns the empty type.
   if ( * this ) return *(fScopeName->fScopeBase);
   return Dummy::Type();
}



//-------------------------------------------------------------------------------
Reflex::Scope Reflex::Scope::DeclaringScope() const {
//-------------------------------------------------------------------------------
   if ( * this ) return fScopeName->fScopeBase->DeclaringScope(); 
   return Dummy::Scope();
}


//-------------------------------------------------------------------------------
Reflex::Member Reflex::Scope::FunctionMemberByName( const std::string & name,
                                                    const Type & signature,
                                                    unsigned int modifiers_mask) const {
//------------------------------------------------------------------------------- 
// Return a function member by it's name, qualified by it's signature type.
   if ( * this ) return fScopeName->fScopeBase->FunctionMemberByName( name, signature, modifiers_mask ); 
   return Dummy::Member();
}


//-------------------------------------------------------------------------------
Reflex::Member Reflex::Scope::FunctionMemberByNameAndSignature( const std::string & name,
                                                                const Type & signature,
                                                                unsigned int modifiers_mask) const {
//------------------------------------------------------------------------------- 
// Return a function member by it's name, qualified by it's signature type.
   if ( * this ) return fScopeName->fScopeBase->FunctionMemberByNameAndSignature( name, signature, modifiers_mask ); 
   return Dummy::Member();
}


//-------------------------------------------------------------------------------
Reflex::Scope Reflex::Scope::GlobalScope() {
//-------------------------------------------------------------------------------
  return Internal::ScopeBase::GlobalScope();
}


//-------------------------------------------------------------------------------
bool Reflex::Scope::IsTopScope() const {
//-------------------------------------------------------------------------------
   if ( * this ) return fScopeName->fScopeBase->IsTopScope(); 
   return false;
}


//-------------------------------------------------------------------------------
Reflex::Member 
Reflex::Scope::LookupMember( const std::string & nam ) const {
//-------------------------------------------------------------------------------
// Lookup a member from this scope.
   if ( * this ) return fScopeName->fScopeBase->LookupMember( nam, *this );
   return Dummy::Member();
}


//-------------------------------------------------------------------------------
Reflex::Type
Reflex::Scope::LookupType( const std::string & nam ) const {
//-------------------------------------------------------------------------------
// Lookup a type from this scope.
   if ( * this ) return fScopeName->fScopeBase->LookupType( nam, *this );
   return Dummy::Type();
}


//-------------------------------------------------------------------------------
Reflex::Scope
Reflex::Scope::LookupScope( const std::string & nam ) const {
//-------------------------------------------------------------------------------
// Lookup a scope from this scope.
   if ( * this ) return fScopeName->fScopeBase->LookupScope( nam, *this );
   return Dummy::Scope();
}


//-------------------------------------------------------------------------------
Reflex::Member 
Reflex::Scope::MemberByName( const std::string & name,
                             const Type & signature ) const {
//-------------------------------------------------------------------------------
// Return a member in this scope, looked up by name and signature (for functions)
   if ( * this ) return fScopeName->fScopeBase->MemberByName(name, signature); 
   return Dummy::Member();
}


//-------------------------------------------------------------------------------
std::string Reflex::Scope::Name( unsigned int mod ) const {
//-------------------------------------------------------------------------------
// Return the name of this scope, scoped if requested.
   std::string buf;
   return Name(buf, mod);
}


//-------------------------------------------------------------------------------
const std::string& Reflex::Scope::Name(std::string& buf, unsigned int mod ) const {
//-------------------------------------------------------------------------------
// Return the name of this scope, scoped if requested.
   if (*this)
      return fScopeName->fScopeBase->Name(buf, mod);
   if (fScopeName) {
      if (mod & SCOPED) return (buf = fScopeName->Name());
      else              return (buf = Tools::GetBaseName(fScopeName->Name()));
   }
   return (buf = "");
}


//-------------------------------------------------------------------------------
const char * Reflex::Scope::Name_c_str() const {
//-------------------------------------------------------------------------------
   if ( fScopeName ) return fScopeName->Name_c_str();
   return "";
}


//-------------------------------------------------------------------------------
Reflex::PropertyList Reflex::Scope::Properties() const {
//-------------------------------------------------------------------------------
   if ( * this ) return fScopeName->fScopeBase->Properties();
   return Dummy::PropertyList();
}


//-------------------------------------------------------------------------------
Reflex::TYPE Reflex::Scope::ScopeType() const {
//-------------------------------------------------------------------------------
   if ( * this ) return fScopeName->fScopeBase->ScopeType(); 
   return UNRESOLVED;
}


//-------------------------------------------------------------------------------
std::string Reflex::Scope::ScopeTypeAsString() const {
//-------------------------------------------------------------------------------
   if ( * this ) return fScopeName->fScopeBase->ScopeTypeAsString(); 
   return "UNRESOLVED";
}


//-------------------------------------------------------------------------------
size_t Reflex::Scope::SubScopeLevel() const {
//-------------------------------------------------------------------------------
   if ( * this ) return fScopeName->fScopeBase->SubScopeLevel();
   return 0;
}


//-------------------------------------------------------------------------------
const Reflex::Internal::ScopeBase * Reflex::Scope::ToScopeBase() const {
//-------------------------------------------------------------------------------
   if ( * this ) return fScopeName->fScopeBase;
   return 0;
}


//-------------------------------------------------------------------------------
void Reflex::Scope::AddMember( const Member & dm ) const {
//-------------------------------------------------------------------------------
// Add data member dm to this scope.
   if ( * this) fScopeName->fScopeBase->AddMember( dm );
}


//-------------------------------------------------------------------------------
void Reflex::Scope::AddMember( const char * name,
                               const Type & type,
                               size_t offset,
                               unsigned int modifiers ) const {
//-------------------------------------------------------------------------------
// Add data member to this scope.
   if ( * this ) fScopeName->fScopeBase->AddMember( name, 
                                                    type, 
                                                    offset, 
                                                    modifiers );
}


//-------------------------------------------------------------------------------
void Reflex::Scope::AddMember( const char * nam,
                               const Type & typ,
                               StubFunction stubFP,
                               void * stubCtx,
                               const char * params,
                               unsigned int modifiers ) const {
//-------------------------------------------------------------------------------
// Add function member to this scope.
   if ( * this ) fScopeName->fScopeBase->AddMember( nam, 
                                                    typ, 
                                                    stubFP, 
                                                    stubCtx, 
                                                    params, 
                                                    modifiers );
}


//-------------------------------------------------------------------------------
void Reflex::Scope::RemoveMember( const Member & dm ) const {
//-------------------------------------------------------------------------------
// Remove data member dm from this scope.
   if ( * this) fScopeName->fScopeBase->RemoveMember( dm );
}


//-------------------------------------------------------------------------------
void Reflex::Scope::AddSubType( const Type & ty ) const {
//-------------------------------------------------------------------------------
// Add sub type ty to this scope.
   if ( * this) fScopeName->fScopeBase->AddSubType( ty );
}


//-------------------------------------------------------------------------------
void Reflex::Scope::AddSubType( const char * type,
                                size_t size,
                                TYPE typeType,
                                const std::type_info & typeInfo,
                                unsigned int modifiers ) const {
//-------------------------------------------------------------------------------
// Add sub type to this scope.
   if ( * this ) fScopeName->fScopeBase->AddSubType( type, 
                                                     size, 
                                                     typeType, 
                                                     typeInfo, 
                                                     modifiers );
}


//-------------------------------------------------------------------------------
void Reflex::Scope::RemoveSubType( const Type & ty ) const {
//-------------------------------------------------------------------------------
// Remove sub type ty from this scope.
   if ( * this) fScopeName->fScopeBase->RemoveSubType( ty );
}


//-------------------------------------------------------------------------------
void Reflex::Scope::AddSubScope( const Scope & sc ) const {
//-------------------------------------------------------------------------------
   if ( * this) fScopeName->fScopeBase->AddSubScope( sc );
}


//-------------------------------------------------------------------------------
void Reflex::Scope::AddSubScope( const char * scope,
                                        TYPE scopeType ) const {
//-------------------------------------------------------------------------------
   if ( * this ) fScopeName->fScopeBase->AddSubScope( scope, scopeType );
}


//-------------------------------------------------------------------------------
void Reflex::Scope::RemoveSubScope( const Scope & sc ) const {
//-------------------------------------------------------------------------------
   if ( * this) fScopeName->fScopeBase->RemoveSubScope( sc );
}


//-------------------------------------------------------------------------------
void Reflex::Scope::AddUsingDirective( const Scope & ud ) const {
//-------------------------------------------------------------------------------
   if ( * this ) fScopeName->fScopeBase->AddUsingDirective( ud );
}


//-------------------------------------------------------------------------------
void Reflex::Scope::RemoveUsingDirective( const Scope & ud ) const {
//-------------------------------------------------------------------------------
   if ( * this ) fScopeName->fScopeBase->RemoveUsingDirective( ud );
}

//-------------------------------------------------------------------------------
void Reflex::Scope::AddMember( const MemberTemplate & mt ) const {
//-------------------------------------------------------------------------------
// Add member template mt to this scope.
   if ( * this ) fScopeName->fScopeBase->AddMember( mt );
}


//-------------------------------------------------------------------------------
void Reflex::Scope::RemoveMember( const MemberTemplate & mt ) const {
//-------------------------------------------------------------------------------
// Remove member template mt from this scope.
   if ( * this ) fScopeName->fScopeBase->RemoveMember( mt );
}


//-------------------------------------------------------------------------------
void Reflex::Scope::AddSubTypeTemplate( const TypeTemplate & tt ) const {
//-------------------------------------------------------------------------------
// Add type template tt to this scope.
   if ( * this ) fScopeName->fScopeBase->AddSubTypeTemplate( tt );
}


//-------------------------------------------------------------------------------
void Reflex::Scope::RemoveSubTypeTemplate( const TypeTemplate & tt ) const {
//-------------------------------------------------------------------------------
// Remove type template tt from this scope.
   if ( * this ) fScopeName->fScopeBase->RemoveSubTypeTemplate( tt );
}


//-------------------------------------------------------------------------------
void Reflex::Scope::GenerateDict( DictionaryGenerator & generator) const {
//-------------------------------------------------------------------------------
// Generate Dictionary information about itself.
   if ( * this ) fScopeName->fScopeBase->GenerateDict(generator);
}


//-------------------------------------------------------------------------------
void Reflex::Scope::Unload() const {
//-------------------------------------------------------------------------------
// Unload a scope, i.e. delete the ScopeName's ScopeBase object.
   if ( * this ) delete fScopeName->fScopeBase;
}


#ifdef REFLEX_CINT_MERGE
bool Reflex::Scope::operator&&(const Scope &right) const
{ return operator bool() && (bool)right; }
bool Reflex::Scope::operator&&(const Type &right) const 
{ return operator bool() && (bool)right; }
bool Reflex::Scope::operator&&(const Member &right) const 
{ return operator bool() && (bool)right; }
bool Reflex::Scope::operator||(const Scope &right) const 
{ return operator bool() && (bool)right; }
bool Reflex::Scope::operator||(const Type &right) const 
{ return operator bool() && (bool)right; }
bool Reflex::Scope::operator||(const Member &right) const 
{ return operator bool() || (bool)right; }
#endif
