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

#include "ScopeBase.h"

#include "Reflex/Scope.h"
#include "Reflex/Type.h"
#include "Reflex/Tools.h"
#include "Reflex/Catalog.h"
#include "Reflex/TypeTemplate.h"
#include "Reflex/DictionaryGenerator.h"

#include "OwnedMember.h"
#include "ScopeName.h"
#include "OwnedMemberTemplate.h"
#include "InternalTools.h"
#include "Class.h"
#include "Namespace.h"
#include "DataMember.h"
#include "FunctionMember.h"
#include "Union.h"
#include "Enum.h"
#include "NameLookup.h"
#include "PropertyListImpl.h"

//-------------------------------------------------------------------------------
Reflex::Internal::ScopeBase::ScopeBase(const char * scope,
                                       ETYPE scopeType,
                                       Catalog catalog)
   : fScopeName(0),
     fScopeType(scopeType),
     fPropertyList(OwnedPropertyList(new PropertyListImpl())),
     fBasePosition(Tools::GetBasePosition(scope)) {
//-------------------------------------------------------------------------------
   // Construct the dictionary information for a scope.
   std::string sname(scope);

   std::string declScope = "";
   std::string currScope = sname;

   if (fBasePosition) {
      declScope = sname.substr(0, fBasePosition-2);
      currScope = std::string(sname, fBasePosition);
   }

   // Construct Scope
   Scope scopePtr = catalog.ScopeByName(sname);
   if (scopePtr.Id() == 0) { 
      // create a new Scope
      fScopeName = new ScopeName(scope, this, catalog); 
   }
   else {
      fScopeName = (ScopeName*)scopePtr.Id();
      fScopeName->fScopeBase = this;
   }

   Scope declScopePtr = catalog.ScopeByName(declScope);
   if (! declScopePtr) {
      if (scopeType == kETNamespace) declScopePtr = (new Namespace(declScope.c_str(), catalog))->ThisScope();
      else                         declScopePtr = (new ScopeName(declScope.c_str(), 0, catalog))->ThisScope();
   }

   // Set declaring Scope and sub-scopes
   fDeclaringScope = declScopePtr; 
   if (fDeclaringScope)  fDeclaringScope.AddSubScope(this->ThisScope());
}


//-------------------------------------------------------------------------------
Reflex::Internal::ScopeBase::ScopeBase(Catalog catalog) 
   : fScopeName(0),
     fScopeType(kETNamespace),
     fDeclaringScope(catalog.__NIRVANA__()),
     fPropertyList(OwnedPropertyList(new PropertyListImpl())),
     fBasePosition(0) {
//-------------------------------------------------------------------------------
   // Default constructor for the ScopeBase (used at init time for the global scope)
   fScopeName = new ScopeName("", this, catalog);
   fPropertyList.AddProperty("Description", "global namespace");
}


//-------------------------------------------------------------------------------
Reflex::Internal::ScopeBase::~ScopeBase() {
//-------------------------------------------------------------------------------
   // Destructor.

   for (OrdOwnedMemberCont_t::iterator it = fMembers.Begin(); it; ++it) {
      if (*it && it->DeclaringScope() == ThisScope())
         (const_cast<OwnedMember&>(*it)).Delete();
   }

   // Informing Scope that I am going away
   if (fScopeName->fScopeBase == this) fScopeName->fScopeBase = 0;

   // Informing declaring Scope that I am going to do away
   if (fDeclaringScope) {
      fDeclaringScope.RemoveSubScope(ThisScope());
   }

   fPropertyList.Delete();
}


//-------------------------------------------------------------------------------
Reflex::Internal::ScopeBase::operator Reflex::Scope () const {
//-------------------------------------------------------------------------------
   // Conversion operator to Scope.
   return ThisScope();
}


//-------------------------------------------------------------------------------
Reflex::Internal::ScopeBase::operator Reflex::Type () const {
//-------------------------------------------------------------------------------
   // Conversion operator to Type.
   switch (fScopeType) {
   case kETClass:
   case kETStruct:
   case kETTypeTemplateInstance:
   case kETUnion:
   case kETEnum:
      return (dynamic_cast<const TypeBase*>(this))->ThisType();
   default:
      return Dummy::Type();
   }
}


//-------------------------------------------------------------------------------
Reflex::Member
Reflex::Internal::ScopeBase::FunctionMemberByName(const std::string & name,
                                               const Type & signature,
                                               unsigned int modifiers_mask) const {
//-------------------------------------------------------------------------------
   // Return function member by name and signature.
   for (OrdMemberCont_t::iterator it = fFunctionMembers.Begin(); it; ++it) {
      if (it->Name() == name
         && (!signature
            || signature.IsSignatureEquivalentTo(it->TypeOf(), modifiers_mask))
         )
         return *it;
   }
   return Dummy::Member();
}


//-------------------------------------------------------------------------------
Reflex::Scope
Reflex::Internal::ScopeBase::GlobalScope() {
//-------------------------------------------------------------------------------
   // Return a ref to the global scope.
   static Scope sGlobal = Catalog::Instance().ScopeByName("");
   return sGlobal;
}


//-------------------------------------------------------------------------------
void
Reflex::Internal::ScopeBase::HideName() const {
//-------------------------------------------------------------------------------
// Append the string " @HIDDEN@" to a scope name.
   fScopeName->HideName();
}


//-------------------------------------------------------------------------------
Reflex::Catalog
Reflex::Internal::ScopeBase::InCatalog() const {
//-------------------------------------------------------------------------------
// Retrieve the Catalog containing the type.
   return fScopeName->InCatalog();
}


//-------------------------------------------------------------------------------
bool
Reflex::Internal::ScopeBase::IsTopScope() const {
//-------------------------------------------------------------------------------
   // Check if this scope is the top scope.
   if (fDeclaringScope == InCatalog().__NIRVANA__()) return true;
   return false;
}


//-------------------------------------------------------------------------------
Reflex::Member 
Reflex::Internal::ScopeBase::LookupMember(const std::string & nam,
                                       const Scope & current) const {
//------------------------------------------------------------------------------- 
   // Lookup a member name from this scope.
   return NameLookup::LookupMember(nam, current);
}


//-------------------------------------------------------------------------------
Reflex::Type
Reflex::Internal::ScopeBase::LookupType(const std::string & nam,
                                     const Scope & current) const {
//-------------------------------------------------------------------------------
   // Lookup a type name from this scope.
   return NameLookup::LookupType(nam, current);
}


//-------------------------------------------------------------------------------
Reflex::Scope
Reflex::Internal::ScopeBase::LookupScope(const std::string & nam,
                                      const Scope & current) const {
//-------------------------------------------------------------------------------
   // Lookup a scope name from this scope.
   return NameLookup::LookupScope(nam, current);
}


//-------------------------------------------------------------------------------
Reflex::Member 
Reflex::Internal::ScopeBase::MemberByName(const std::string & name,
                                       const Type & signature) const {
//-------------------------------------------------------------------------------
   // Return member by name and signature.
   if (signature) return FunctionMemberByName(name, signature, 0);
   OrdOwnedMemberCont_t::iterator iM = fMembers.Find(name);
   if (iM)
      return *iM;
   return Dummy::Member();
}


//-------------------------------------------------------------------------------
const std::string&
Reflex::Internal::ScopeBase::Name(std::string& buf, unsigned int mod) const {
//-------------------------------------------------------------------------------
   // Return name of this scope.
   if (mod & kScoped)
      buf += fScopeName->Name();
   else
      buf.append(fScopeName->Name(), fBasePosition, std::string::npos);
   return buf;
}


//-------------------------------------------------------------------------------
Reflex::PropertyList
Reflex::Internal::ScopeBase::Properties() const {
//-------------------------------------------------------------------------------
   // Return property list attached to this scope.
   return fPropertyList;
}


//-------------------------------------------------------------------------------
const std::string &
Reflex::Internal::ScopeBase::SimpleName(size_t & pos, 
                                       unsigned int mod) const {
//-------------------------------------------------------------------------------
// Return the name of the type.
   if (mod & kScoped)
      pos = 0;
   else
      pos = fBasePosition;
   return fScopeName->Name();
}


//-------------------------------------------------------------------------------
Reflex::Scope
Reflex::Internal::ScopeBase::ThisScope() const {
//-------------------------------------------------------------------------------
   // Return the scope of this scope base.
   return fScopeName->ThisScope();
}


//-------------------------------------------------------------------------------
std::string
Reflex::Internal::ScopeBase::ScopeTypeAsString() const {
//-------------------------------------------------------------------------------
   // Return the type of the scope as a string.
   const std::string& s = TYPEName(fScopeType);
   if (s.empty()) {
      std::string name;
      return "Scope " + Name(name) + "is not assigned to a SCOPE";
   }
   return s;
}


//-------------------------------------------------------------------------------
size_t
Reflex::Internal::ScopeBase::SubScopeLevel() const {
//-------------------------------------------------------------------------------
   size_t level = 0;
   Scope tmp = ThisScope();
   while (! tmp.IsTopScope()) {
      tmp = tmp.DeclaringScope();
      ++level;
   }
   return level;
}


//-------------------------------------------------------------------------------
void
Reflex::Internal::ScopeBase::AddMember(const char * name,
                                             const Type & type,
                                             size_t offset,
                                             unsigned int modifiers) const {
//-------------------------------------------------------------------------------
   // Add data member to this scope.
   AddMember(Member(new DataMember(name, type, offset, modifiers)));
}


//-------------------------------------------------------------------------------
void
Reflex::Internal::ScopeBase::RemoveMember(const Member & m) const {
//-------------------------------------------------------------------------------
   // Remove member m from this scope.
   Hash_t hash = fMembers.ValueHash(m);
   if (m.Is(kDataMember))
      fDataMembers.Remove(m, hash);
   else
      fFunctionMembers.Remove(m, hash);

   fMembers.Remove(m, hash);
}


//-------------------------------------------------------------------------------
void
Reflex::Internal::ScopeBase::AddMember(const Member & m) const {
//-------------------------------------------------------------------------------
   // Add member m to this scope.
   m.SetScope(ThisScope());
   Hash_t hash = fMembers.ValueHash(m);
   if (m.Is(kDataMember))
      fDataMembers.Insert(m, hash);
   else
      fFunctionMembers.Insert(m, hash);

   fMembers.Insert(m, hash);
}


//-------------------------------------------------------------------------------
void
Reflex::Internal::ScopeBase::AddMember(const char * name,
                                                 const Type & type,
                                                 StubFunction stubFP,
                                                 void * stubCtx,
                                                 const char * params,
                                                 unsigned int modifiers) const {
//-------------------------------------------------------------------------------
   // Add function member to this scope.
   AddMember(Member(new FunctionMember(name, type, stubFP, stubCtx, params, modifiers)));
}


//-------------------------------------------------------------------------------
void
Reflex::Internal::ScopeBase::AddMember(const MemberTemplate & mt) const {
//-------------------------------------------------------------------------------
   // Add member template mt to this scope.
   fMemberTemplates.Insert(mt);
}


//-------------------------------------------------------------------------------
void
Reflex::Internal::ScopeBase::RemoveMember(const MemberTemplate & mt) const {
//-------------------------------------------------------------------------------
   // Remove member template mt from this scope.
   fMemberTemplates.Remove(mt);
}


//-------------------------------------------------------------------------------
void
Reflex::Internal::ScopeBase::AddSubScope(const Scope & subscope) const {
//-------------------------------------------------------------------------------
   // Add sub scope to this scope.
   fSubScopes.Insert(subscope);
}


//-------------------------------------------------------------------------------
void
Reflex::Internal::ScopeBase::AddSubScope(const char * scope,
                                         ETYPE scopeType) const {
//-------------------------------------------------------------------------------
   // Add sub scope to this scope.
   AddSubScope(*(new ScopeBase(scope, scopeType, InCatalog())));
}


//-------------------------------------------------------------------------------
void
Reflex::Internal::ScopeBase::RemoveSubScope(const Scope & subscope) const {
//-------------------------------------------------------------------------------
   // Remove sub scope from this scope.
   fSubScopes.Remove(subscope); 
}


//-------------------------------------------------------------------------------
void
Reflex::Internal::ScopeBase::AddSubType(const Type & ty) const {
//-------------------------------------------------------------------------------
   // Add sub type ty to this scope.
   fSubTypes.Insert(ty);
}


//-------------------------------------------------------------------------------
void
Reflex::Internal::ScopeBase::AddSubType(const char * type,
                                          size_t size,
                                          ETYPE typeType,
                                          const std::type_info & ti,
                                          unsigned int modifiers) const {
//-------------------------------------------------------------------------------
   // Add sub type to this scope.
   TypeBase * tb = 0;
   switch (typeType) {
   case kETClass:
      tb = new Class(type,size,ti, InCatalog(), modifiers);
      break;
   case kETStruct:
      tb = new Class(type,size,ti, InCatalog(),modifiers,kETStruct);
      break;
   case kETEnum:
      tb = new Enum(type,ti, InCatalog(),modifiers);
      break;
   case kETFunction:
      break;
   case kETArray:
      break;
   case kETFundamental:
      break;
   case  kETPointer:
      break;
   case kETPointerToMember:
      break;
   case kETTypedef:
      break;
   case kETUnion:
      tb = new Union(type,size,ti, InCatalog(),modifiers); 
      break;
   default:
      tb = new TypeBase(type, modifiers, size, typeType, ti, InCatalog());
   }
   if (tb) AddSubType(* tb);
}


//-------------------------------------------------------------------------------
void
Reflex::Internal::ScopeBase::RemoveSubType(const Type & ty) const {
//-------------------------------------------------------------------------------
   // Remove sub type ty from this scope.
   fSubTypes.Remove(ty); 
}


//-------------------------------------------------------------------------------
void
Reflex::Internal::ScopeBase::AddSubTypeTemplate(const TypeTemplate & tt) const {
//-------------------------------------------------------------------------------
   // Add sub type template to this scope.
   fTypeTemplates.Insert(tt);
}


//-------------------------------------------------------------------------------
void
Reflex::Internal::ScopeBase::RemoveSubTypeTemplate(const TypeTemplate & tt) const {
//-------------------------------------------------------------------------------
   // Remove sub type template tt from this scope.
   fTypeTemplates.Remove(tt);
}


//-------------------------------------------------------------------------------
void
Reflex::Internal::ScopeBase::AddUsingDirective(const Scope & ud) const {
//-------------------------------------------------------------------------------
   // Add using directive ud to this scope.
   fUsingDirectives.Insert(ud);
}


//-------------------------------------------------------------------------------
void
Reflex::Internal::ScopeBase::AddUsingDeclaration(const Type & ud) const {
//-------------------------------------------------------------------------------
// Adds a using directive of a type to this scope, as in
//   using scope::ud;
// @param ud using declaration to add
   fTypeUsingDeclarations.Insert(ud);
}


//-------------------------------------------------------------------------------
void
Reflex::Internal::ScopeBase::AddUsingDeclaration(const Member & ud) const {
//-------------------------------------------------------------------------------
// Adds a using declaration of a member to this scope, as in
//  using scope::ud;
// @param ud using declaration to add
   fMemberUsingDeclarations.Insert(ud);
}


//-------------------------------------------------------------------------------
void
Reflex::Internal::ScopeBase::RemoveUsingDirective(const Scope & ud) const {
//-------------------------------------------------------------------------------
   // Remove using directive ud from this scope.
   fUsingDirectives.Remove(ud);
}


//-------------------------------------------------------------------------------
void
Reflex::Internal::ScopeBase::RemoveUsingDeclaration(const Type & ud) const {
//-------------------------------------------------------------------------------
// Removes a using declaration from this scope
// @param ud using declaration to remove
   fTypeUsingDeclarations.Remove(ud);
}


//-------------------------------------------------------------------------------
void
Reflex::Internal::ScopeBase::RemoveUsingDeclaration(const Member & ud) const {
//-------------------------------------------------------------------------------
// Removes a using declaration from this scope
// @param ud using declaration to remove
   fMemberUsingDeclarations.Remove(ud);
}


//-------------------------------------------------------------------------------
void
Reflex::Internal::ScopeBase::GenerateDict(DictionaryGenerator & generator) const {
//-------------------------------------------------------------------------------
   // Generate Dictionary information about itself.

   if (generator.Use_recursive()) {
      for(OrdScopeUniqueCont_t::reverse_iterator iSubScope = SubScopes().RBegin();
         iSubScope; ++iSubScope)
         iSubScope->GenerateDict(generator);
   }
}
