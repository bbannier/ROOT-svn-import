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

#include "Union.h"

#include "Reflex/Tools.h"

//______________________________________________________________________________
Reflex::Internal::Union::Union(const char* typ, size_t size, const std::type_info& ti, unsigned int modifiers, ETYPE unionType /*=kUnion*/)
: TypeBase(typ, size, unionType, ti)
, ScopeBase(typ, unionType)
, fModifiers(modifiers)
, fCompleteType(false)
{
// Construct union info.
}

//______________________________________________________________________________
Reflex::Internal::Union::~Union()
{
// Destructor.
}

//______________________________________________________________________________
Reflex::Internal::Union::operator Reflex::Scope() const
{
   return ScopeBase::operator Scope();
}

//______________________________________________________________________________
Reflex::Internal::Union::operator Reflex::Type() const
{
   return TypeBase::operator Type();
}

//______________________________________________________________________________
void
Reflex::Internal::Union::HideName() const
{
//  Hide the union from name lookup; forwards to TypeBase and ScopeBase.
   TypeBase::HideName();
   ScopeBase::HideName();
}

//______________________________________________________________________________
Reflex::Scope
Reflex::Internal::Union::DeclaringScope() const
{
// Return the scope the union is a member of.
   return ScopeBase::DeclaringScope();
}

//______________________________________________________________________________
bool
Reflex::Internal::Union::IsComplete() const
{
   // Return true if this union is complete. I.e. all dictionary information for all
   // data and function member types is available.
   if (!fCompleteType) {
      fCompleteType = true;
   }
   return fCompleteType;
}

//______________________________________________________________________________
const std::string&
Reflex::Internal::Union::Name(std::string& buf, unsigned int mod /*= 0*/) const
{
// Return the name of the union; possible modifiers:
//      *   FINAL     - resolve typedefs
//      *   kScoped    - fully scoped name 
//      *   kQualified - cv, reference qualification 
   return ScopeBase::Name(buf, mod);
}

//______________________________________________________________________________
const std::string&
Reflex::Internal::Union::SimpleName(size_t& pos, unsigned int mod /*=0*/) const
{
// Return the "simple" name of the union (only the left-most part of the scoped name)
// possible modifiers:
//      *   FINAL     - resolve typedefs
//      *   kScoped    - fully scoped name 
//      *   kQualified - cv, reference qualification 
// See ScopeBase::SimpleName().
   return ScopeBase::SimpleName(pos, mod);
}

//______________________________________________________________________________
Reflex::PropertyList
Reflex::Internal::Union::Properties() const
{
// Return the union's list of properties.
   return ScopeBase::Properties();
}

//______________________________________________________________________________
void
Reflex::Internal::Union::AddMember(const Member & m) const
{
// Add data or function member fm to this union
   ScopeBase::AddMember(m);
   if (m.Is(gConstructor)) {
      fConstructors.push_back(m);
   }
   else if (fm.Is(gDestructor)) {
      fDestructor = m;
   }
}

//______________________________________________________________________________
void
Reflex::Internal::Union::AddMember(const char* nam, const Type& typ, StubFunction stubFP, void* stubCtx, const char* params, unsigned int modifiers) const
{
// Add function member to this union.
   ScopeBase::AddMember(nam, typ, stubFP, stubCtx, params, modifiers);
   if (modifiers & kConstructor) {
      fConstructors.push_back(fFunctionMembers[fFunctionMembers.size()-1]);
   }
   // setting the destructor is not needed because it is always provided when building the union
}

//______________________________________________________________________________
inline Reflex::Internal::TypeName*
Reflex::Internal::Union::TypeNameGet() const
{
// Return the TypeName* of this union.
   return fTypeName;
}

