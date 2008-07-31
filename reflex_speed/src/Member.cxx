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

#include "Reflex/Member.h"

#include "Reflex/Scope.h"
#include "Reflex/Type.h"
#include "Reflex/Base.h"
#include "Reflex/PropertyList.h"
#include "Reflex/MemberTemplate.h"
#include "Reflex/Tools.h"

#include "MemberBase.h"

#include "Class.h"

#include <iostream>


//-------------------------------------------------------------------------------
Reflex::Scope
Reflex::Member::DeclaringScope() const {
//-------------------------------------------------------------------------------
   if (*this) return fMemberBase->DeclaringScope();
   return Dummy::Scope();
}


//-------------------------------------------------------------------------------
Reflex::Type
Reflex::Member::DeclaringType() const {
//-------------------------------------------------------------------------------
   if (*this) return fMemberBase->DeclaringScope();
   return Dummy::Type();
}


-------------------------------------------------------------------------------
void
Reflex::Member::Delete() {
//-------------------------------------------------------------------------------
// delete the MemberBase
   delete fMemberBase;
   fMemberBase = 0;
}


//-------------------------------------------------------------------------------
void
Reflex::Member::GenerateDict(DictionaryGenerator & generator) const {
//-------------------------------------------------------------------------------
// Generate Dictionary information about itself.
   if (* this) fMemberBase->GenerateDict(generator);
}
                           

//-------------------------------------------------------------------------------
Reflex::Object
Reflex::Member::Get() const {
//-------------------------------------------------------------------------------
// Get the value of a static member.
   if (fMemberBase) return fMemberBase->Get(Object());
   return Object();
}


//-------------------------------------------------------------------------------
Reflex::Object
Reflex::Member::Get(const Object & obj) const {
//-------------------------------------------------------------------------------
// Get the value of a non static data member.
   if (fMemberBase) return fMemberBase->Get(obj);
   return Object();
}


//-------------------------------------------------------------------------------
Reflex::Object 
Reflex::Member::Invoke(const Object & obj,
                        const std::vector < void * > & paramList) const {
//-------------------------------------------------------------------------------
// Invoke a non static data member.
   if (fMemberBase) return fMemberBase->Invoke(obj, paramList);
   return Object();
}


//-------------------------------------------------------------------------------
Reflex::Object 
Reflex::Member::Invoke(const std::vector < void * > & paramList) const {
//-------------------------------------------------------------------------------
// Invoke a static data member.
   if (fMemberBase) return fMemberBase->Invoke(paramList);
   return Object();
}


//-------------------------------------------------------------------------------
Reflex::ETYPE
Reflex::Member::MemberType() const {
//-------------------------------------------------------------------------------
   if (*this) return fMemberBase->MemberType();
   return kUnresolved;
}


//-------------------------------------------------------------------------------
std::string
Reflex::Member::MemberTypeAsString() const {
//-------------------------------------------------------------------------------
   if (*this) return fMemberBase->MemberTypeAsString();
   return "";
}


//-------------------------------------------------------------------------------
const std::string&
Reflex::Member::Name(std::string& buf, unsigned int mod) const {
//-------------------------------------------------------------------------------
   if (*this) return fMemberBase->Name(buf, mod);
   return buf;
}


//-------------------------------------------------------------------------------
size_t
Reflex::Member::Offset() const {
//-------------------------------------------------------------------------------
   if (*this) return fMemberBase->Offset();
   return 0;
}


//-------------------------------------------------------------------------------
Reflex::PropertyList
Reflex::Member::Properties() const {
//-------------------------------------------------------------------------------
   if (*this) return fMemberBase->Properties();
   return Dummy::PropertyList();
}


//-------------------------------------------------------------------------------
void
Reflex::Member::Set(const Object & instance,
                                const void * value) const {
//-------------------------------------------------------------------------------
// Set a non static data member.
   if (fMemberBase) fMemberBase->Set(instance, value);
}


//-------------------------------------------------------------------------------
void
Reflex::Member::SetScope(const Scope & sc) const  {
//-------------------------------------------------------------------------------
   if (*this) fMemberBase->SetScope(sc);
}


//-------------------------------------------------------------------------------
void *
Reflex::Member::Stubcontext() const {
//-------------------------------------------------------------------------------
   if (*this) return fMemberBase->Stubcontext();
   return 0;
}


//-------------------------------------------------------------------------------
Reflex::StubFunction
Reflex::Member::Stubfunction() const {
//-------------------------------------------------------------------------------
   if (*this) return fMemberBase->Stubfunction();
   return 0;
}


//-------------------------------------------------------------------------------
Reflex::MemberTemplate
Reflex::Member::TemplateFamily() const {
//-------------------------------------------------------------------------------
   if (* this) return fMemberBase->TemplateFamily();
   return Dummy::MemberTemplate();
}


//-------------------------------------------------------------------------------
Reflex::Type
Reflex::Member::TypeOf() const {
//-------------------------------------------------------------------------------
   if (*this) return fMemberBase->TypeOf();
   return Dummy::Type();
}


//-------------------------------------------------------------------------------
void
Reflex::Member::UpdateFunctionParameterNames(const char* parameters) {
//-------------------------------------------------------------------------------
   if (*this) return fMemberBase->UpdateFunctionParameterNames(parameters);
}


#ifdef REFLEX_CINT_MERGE
bool
Reflex::Member::operator&&(const Scope &right) const
{ return operator bool() && (bool)right; }
bool
Reflex::Member::operator&&(const Type &right) const 
{ return operator bool() && (bool)right; }
bool
Reflex::Member::operator&&(const Member &right) const 
{ return operator bool() && (bool)right; }
bool
Reflex::Member::operator||(const Scope &right) const 
{ return operator bool() && (bool)right; }
bool
Reflex::Member::operator||(const Type &right) const 
{ return operator bool() && (bool)right; }
bool
Reflex::Member::operator||(const Member &right) const 
{ return operator bool() && (bool)right; }
#endif
