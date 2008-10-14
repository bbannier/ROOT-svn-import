// @(#)root/reflex:$Id: Constructable.cxx 25003 2008-07-31 16:11:41Z axel $
// Author: Axel Naumann 2008

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

#include "Constructable.h"

#include "Reflex/Object.h"
#include "Reflex/Type.h"
#include "Reflex/Catalog.h"

#include <iostream>
#include <sstream>


/*/-------------------------------------------------------------------------------
  Reflex::Object Reflex::Internal::Constructable::Construct(const Type & signature,
                                                       const std::vector < Object > & args,
                                                       void * mem) const {
//-------------------------------------------------------------------------------
  static Type defSignature = Type::ByName("void (void)");
  Type signature2 = signature;

  Member constructor = Member();
  if (!signature &&  fConstructors.size() > 1)
  signature2 = defSignature;

  for (size_t i = 0; i < fConstructors.size(); ++ i) {
  if (!signature2 || fConstructors[i].TypeOf().Id() == signature2.Id()) {
  constructor = fConstructors[i];
  break;
  }
  }

  if (constructor.TypeOf()) {
  // no memory Address passed -> Allocate memory for class
  if (mem == 0) mem = Allocate();
  Object obj = Object(TypeOf(), mem);
  constructor.Invoke(obj, args);
  return obj;
  }
  else {
  throw RuntimeError("No suitable constructor found");
  }
  }
*/


//-------------------------------------------------------------------------------
Reflex::Object Reflex::Internal::Constructable::Construct(const Type & sig,
                                        const std::vector < void * > & args,
                                        void * mem) const
{
//-------------------------------------------------------------------------------
// Construct an object of this type. The signature of the constructor function
// can be given as the first argument. Furhter arguments are a vector of memory
// addresses for non default constructors and a memory address for in place construction.
   static Type defSignature = ScopeBase::InCatalog().ByName("void (void)");
   Type signature = (!sig &&  fConstructors.size() > 1) ? defSignature : sig;

   for (size_t i = 0; i < fConstructors.size(); ++ i) {
      if (!signature || fConstructors[i].TypeOf().Id() == signature.Id()) {
         Member constructor = fConstructors[i];
         if (mem == 0) mem = Allocate();
         Object obj = Object(ThisType(), mem);
         constructor.Invoke(obj, args);
         return obj;
      }
   }
   std::stringstream s;
   s << "No suitable constructor found with signature '" << signature.Name() << "'";
   throw RuntimeError(s.str());
}


//-------------------------------------------------------------------------------
void Reflex::Internal::Constructable::Destruct(void * instance,
                             bool dealloc) const
{
//-------------------------------------------------------------------------------
// Call the destructor for this class type on a memory address (instance). Deallocate
// memory if dealloc = true (i.e. default).
   if (! fDestructor.TypeOf()) {
      // destructor for this class not yet revealed
      for (OrdMemberCont_t::iterator i = FunctionMembers().Begin(); i; ++i) {
         Member fm = *i;
         // constructor found Set the cache pointer
         if (fm.Is(kDestructor)) {
            fDestructor = fm;
            break;
         }
      }
   }
   if (fDestructor.TypeOf()) {
      // we found a destructor -> Invoke it
      Object dummy = Object(Type(), instance);
      fDestructor.Invoke(dummy);
      // if deallocation of memory wanted
      if (dealloc) {
         Deallocate(instance);
      }
   }
   else {
      // this class has no destructor defined we call the operator delete on it
      ::operator delete(instance);
   }
}


//-------------------------------------------------------------------------------
void Reflex::Internal::Constructable::AddMember(const Member & m) const
{
//-------------------------------------------------------------------------------
// Add member m to this class
   ScopeBase::AddMember(m);
   if (m.Is(kConstructor))      fConstructors.push_back(m);
   else if (m.Is(kDestructor)) fDestructor = m;
}


//-------------------------------------------------------------------------------
void Reflex::Internal::Constructable::AddMember(const char * nam,
                                      const Type & typ,
                                      StubFunction stubFP,
                                      void * stubCtx,
                                      const char * params,
                                      unsigned int modifiers) const
{
//-------------------------------------------------------------------------------
// Add member to this class
   ScopeBase::AddMember(nam, typ, stubFP, stubCtx, params, modifiers);
   if (modifiers & kEDConstructor)
      fConstructors.push_back(fFunctionMembers.Last());
   // setting the destructor is not needed because it is always provided when building the class
}


//-------------------------------------------------------------------------------
void Reflex::Internal::Constructable::RemoveMember(const Member & m) const
{
//-------------------------------------------------------------------------------
// Remove member from this class.
   if (m.Is(kConstructor)) {
      std::vector<Member>::iterator iCtor
         = std::find(fConstructors.begin(), fConstructors.end(), m);
      if (iCtor != fConstructors.end())
         fConstructors.erase(iCtor);
   } else if (m.Is(kDestructor) && m == fDestructor) {
      fDestructor = Member();
   }
   ScopeBase::RemoveMember(m);
}
