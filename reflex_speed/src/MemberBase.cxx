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

#include "MemberBase.h"

#include "Reflex/Scope.h"
#include "Reflex/Type.h"
#include "Reflex/Base.h"
#include "Reflex/Object.h"
#include "Reflex/DictionaryGenerator.h"
#include "Reflex/Tools.h"

#include "OwnedMember.h"
#include "OwnedPropertyList.h"
#include "Class.h"
#include "PropertyListImpl.h"

//-------------------------------------------------------------------------------
Reflex::Internal::MemberBase::MemberBase(const char *  name,
                                      const Type &  type,
                                      ETYPE          memberType,
                                      unsigned int  modifiers)
//-------------------------------------------------------------------------------
   : fType(type, modifiers & (kConst | kVolatile | kReference) , Type::kAppend),
     fModifiers(modifiers),
     fName(name),
     fScope(Scope()),
     fMemberType(memberType),
     fPropertyList(OwnedPropertyList(new PropertyListImpl())) {
// Construct the dictionary info for a member
   fThisMember = new Member(this);
}


//-------------------------------------------------------------------------------
Reflex::Internal::MemberBase::~MemberBase() {
//-------------------------------------------------------------------------------
// Destructor.
   delete fThisMember;
   fPropertyList.Delete();
}


//-------------------------------------------------------------------------------
Reflex::Internal::MemberBase::operator Reflex::Member () const {
//-------------------------------------------------------------------------------
// Conversion operator to Member.
   return *fThisMember;
}


//-------------------------------------------------------------------------------
void *
Reflex::Internal::MemberBase::CalculateBaseObject(const Object & obj) const {
//-------------------------------------------------------------------------------
// Return the object address a member lives in.
   char * mem = (char*)obj.Address();
   // check if its a dummy object 
   Type cl = obj.TypeOf();
   // if the object type is not implemented return the Address of the object
   if (! cl) return mem; 
   if (cl.Is(gClassOrStruct)) {
      if (DeclaringScope() && (cl.Id() != (dynamic_cast<const Class*>(DeclaringScope().ToScopeBase()))->ThisType().Id())) {
         // now we know that the Member type is an inherited one
         std::vector < OffsetFunction > basePath = (dynamic_cast<const Class*>(cl.ToTypeBase()))->PathToBase(DeclaringScope());
         if (basePath.size()) {
            // there is a path described from the object to the class containing the Member
            std::vector < OffsetFunction >::iterator pIter;
            for (pIter = basePath.begin(); pIter != basePath.end(); ++pIter) {
               mem += (*pIter)(mem);
            }
         }
         else {
            std::string name;
            throw RuntimeError(std::string(": ERROR: There is no path available from class ")
                               + cl.Name(kScoped) + " to " + Name(name, kScoped));
         }
      }
   }
   else {
      throw RuntimeError(std::string("Object ") + cl.Name(kScoped) + " does not represent a class");
   }
   return (void*)mem;
}



//-------------------------------------------------------------------------------
Reflex::Scope
Reflex::Internal::MemberBase::DeclaringScope() const {
//-------------------------------------------------------------------------------
// Return the scope the member lives in.
   return fScope;
}


//-------------------------------------------------------------------------------
Reflex::Type
Reflex::Internal::MemberBase::DeclaringType() const {
//-------------------------------------------------------------------------------
// Return the type the member lives in.
   return DeclaringScope();
}


//-------------------------------------------------------------------------------
std::string
Reflex::Internal::MemberBase::MemberTypeAsString() const {
//-------------------------------------------------------------------------------
// Remember type of the member as a string.
   switch (fMemberType) {
   case kDataMember:
      return "DataMember";
      break;
   case kFunctionMember:
      return "FunctionMember";
      break;
   default:
      {
         std::string name;
         return Reflex::Argv0() + ": ERROR: Member " + Name(name, kScoped) +
            " has no species associated";
      }
   }
}

//-------------------------------------------------------------------------------
Reflex::PropertyList
Reflex::Internal::MemberBase::Properties() const {
//-------------------------------------------------------------------------------
// Return the property list attached to this member.
   return fPropertyList;
}


//-------------------------------------------------------------------------------
void
Reflex::Internal::MemberBase::GenerateDict(DictionaryGenerator & /* generator */) const {
//-------------------------------------------------------------------------------
// Generate Dictionary information about itself.
}
