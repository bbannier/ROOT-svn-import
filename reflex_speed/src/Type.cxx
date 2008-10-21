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

#include "Reflex/Type.h"

#include "Reflex/Catalog.h"
#include "Reflex/Container.h"
#include "Reflex/Object.h"
#include "Reflex/Scope.h"
#include "Reflex/TypeTemplate.h"
#include "Reflex/PropertyList.h"

#include "TypeName.h"
#include "TypeBase.h"

#include "Reflex/Tools.h"


//-------------------------------------------------------------------------------
Reflex::Type::operator bool () const {
//-------------------------------------------------------------------------------
   if (this->fTypeName && this->fTypeName->ToTypeBase()) return true;
   return false;
}


//-------------------------------------------------------------------------------
Reflex::Type::operator Reflex::Scope () const {
//-------------------------------------------------------------------------------
// Conversion operator to Scope.
   if (* this) return *(fTypeName->ToTypeBase());
   return Dummy::Scope();
}


//-------------------------------------------------------------------------------
void *
Reflex::Type::Allocate() const {
//-------------------------------------------------------------------------------
   if (* this) return fTypeName->ToTypeBase()->Allocate();
   return 0;
}


//-------------------------------------------------------------------------------
size_t
Reflex::Type::ArrayLength() const {
//-------------------------------------------------------------------------------
   if (* this) return fTypeName->ToTypeBase()->ArrayLength();
   return 0;
}


//-------------------------------------------------------------------------------
const Reflex::OrderedContainer<Reflex::Base>
Reflex::Type::Bases() const {
//-------------------------------------------------------------------------------
   if (* this) return fTypeName->ToTypeBase()->Bases(); 
   return Dummy::EmptyContainer();
}


//-------------------------------------------------------------------------------
Reflex::Object
Reflex::Type::CastObject(const Type & to,
                          const Object & obj) const {
//-------------------------------------------------------------------------------
// Cast the current type to "to" using the object obj.
   if (* this) return fTypeName->ToTypeBase()->CastObject(to, obj);
   return Dummy::Object();
}


//-------------------------------------------------------------------------------
Reflex::Object
Reflex::Type::Construct(const Type & signature,
                         const std::vector < void * > & values, 
                         void * mem) const {
//-------------------------------------------------------------------------------
// Construct this type and return it as an object. Signature can be used for overloaded
// constructors. Values is a collection of memory addresses of paramters. Mem the memory
// address for in place construction.
   if (* this) return fTypeName->ToTypeBase()->Construct(signature, 
                                                         values, 
                                                         mem); 
   return Dummy::Object();
}


//-------------------------------------------------------------------------------
void
Reflex::Type::Deallocate(void * instance) const {
//-------------------------------------------------------------------------------
   if (* this) fTypeName->ToTypeBase()->Deallocate(instance); 
}


//-------------------------------------------------------------------------------
Reflex::Scope
Reflex::Type::DeclaringScope() const {
//-------------------------------------------------------------------------------
   if (* this) return fTypeName->ToTypeBase()->DeclaringScope();
   return Dummy::Scope();
}


//-------------------------------------------------------------------------------
void
Reflex::Type::Destruct(void * instance, 
                        bool dealloc) const {
//-------------------------------------------------------------------------------
   if (* this) fTypeName->ToTypeBase()->Destruct(instance, dealloc); 
}


//-------------------------------------------------------------------------------
Reflex::Type
Reflex::Type::DynamicType(const Object & obj) const {
//-------------------------------------------------------------------------------
// Return the dynamic type of this type.
   if (* this) return fTypeName->ToTypeBase()->DynamicType(obj);
   return Dummy::Type();
}


//-------------------------------------------------------------------------------
Reflex::Type
Reflex::Type::FinalType() const {
//-------------------------------------------------------------------------------
   if (* this) return Reflex::Type(fTypeName->ToTypeBase()->FinalType(), fModifiers, kAppend);
   return *this;
}


//-------------------------------------------------------------------------------
const Reflex::OrderedContainer<Reflex::Type>
Reflex::Type::FunctionParameters() const {
//-------------------------------------------------------------------------------
   if (* this) return fTypeName->ToTypeBase()->FunctionParameters();
   return Dummy::EmptyContainer();
}


//-------------------------------------------------------------------------------
bool
Reflex::Type::DetermineEquivalence(const Type & typ, unsigned int modifiers_mask, bool alsoReturn) const {
//-------------------------------------------------------------------------------
// Check if two types are equivalent. It will compare the information of the type
// depending on the TypeType. If the type is a fuction and alsoReturn is set,
// the functions' return types are compared, too.

   if (*this == typ) return true;

   Type t1 = *this;
   Type t2 = typ;

   unsigned int mod1 = t1.fModifiers | modifiers_mask;
   unsigned int mod2 = t2.fModifiers | modifiers_mask;

   while (t1.Is(kTypedef)) { 
      t1 = t1.ToType();
      mod1 |= t1.fModifiers;
   }
   while (t2.Is(kTypedef)) {
      t2 = t2.ToType();
      mod2 |= t2.fModifiers;
   }

   if (mod1 != mod2)
      return false;
   if (t1.TypeType() != t2.TypeType())
      return false;

   if (t1.Is(kArray) && t1.ArrayLength() != t2.ArrayLength())
      return false;

   static AttributesExpression needToType(kPointer || kPointerToMember || kArray);

   if (t1.Is(needToType))
      // set alsoReturn to true because we know it's not a function and void(*)() != int(*)()
      return t1.ToType().DetermineEquivalence(t2.ToType(),modifiers_mask, true);

   if (!t1.Is(kFunction))
      return t1.fTypeName == t2.fTypeName;
   else {
      if (alsoReturn)
         if (!t1.ReturnType().IsEquivalentTo(t2.ReturnType(),modifiers_mask))
            return false;

      if (t1.FunctionParameters().Size() != t2.FunctionParameters().Size())
         return false;

      Type_Iterator pi1 = t1.FunctionParameters().Begin();
      Type_Iterator pi2 = t2.FunctionParameters().Begin();
      for (; pi1 != t1.FunctionParameters().End(),  pi2 != t2.FunctionParameters().End(); 
            ++pi1, ++pi2) {

         // set alsoReturn to true because we know it's not a function and void(*)() != int(*)()
         if (!pi1->DetermineEquivalence(*pi2, modifiers_mask, true))
            return false;
      }
      return true;
   }
   return false;
}


bool Reflex::Type::HasBase(const Type & cl) const {
//-------------------------------------------------------------------------------
   // Return base info if type has base cl.
   if (* this) return fTypeName->ToTypeBase()->HasBase(cl);
   return false;
}


//-------------------------------------------------------------------------------
bool
Reflex::Type::Is(const AttributesExpression& descr) const {
//-------------------------------------------------------------------------------
// Test type for properties.
   if (*this) return fTypeName->ToTypeBase()->Is(descr);
   return false;
}


//-------------------------------------------------------------------------------
std::string
Reflex::Type::Name(unsigned int mod) const {
//-------------------------------------------------------------------------------
// Return the name of the type (qualified and scoped if requested)
   std::string buf;
   return Name(buf, mod);
}


//-------------------------------------------------------------------------------
const std::string&
Reflex::Type::Name(std::string& buf, unsigned int mod) const {
//-------------------------------------------------------------------------------
// Return the name of the type (qualified and scoped if requested)

   static const std::string sCVcv = "const volatile";
   static const std::string sCVc  = "const";
   static const std::string sCVv  = "volatile";

   if ((!(mod & kQualified) || !fModifiers) && (*this)) { 
      // most common case
      return fTypeName->ToTypeBase()->Name(buf, mod);
   }

   const std::string *cv = 0;

   /** apply qualifications if wanted */
   if (mod & kQualified) {
      if (Is(kConst && kVolatile)) cv = &sCVcv;
      else if (Is(kConst))         cv = &sCVc;
      else if (Is(kVolatile))      cv = &sCVv;
   }

   bool isPtrOrFunc = Is(kPointer || kFunction);

   /** if At is not a pointer qualifiers can be put before */
   if (cv && !isPtrOrFunc)
      buf += *cv + " ";
  
   /** use implemented names if available */
   if (*this)
      fTypeName->ToTypeBase()->Name(buf, mod);
   /** otherwise use the Internal::TypeName */
   else {
      if (fTypeName) {
         /** unscoped At Name */
         if (mod & kScoped)
            buf += fTypeName->Name();
         else
            buf += Tools::GetBaseName(fTypeName->Name());
      } 
      else { 
         return buf;
      }
   }

   /** if At is a pointer qualifiers have to be after At */
   if (cv && isPtrOrFunc)
      buf += " " + *cv;

   /** apply reference if qualifications wanted */
   if ((mod & kQualified) && Is(kReference))
      buf += "&";

   return buf;
}

//-------------------------------------------------------------------------------
Reflex::Scope
Reflex::Type::PointerToMemberScope() const {
//-------------------------------------------------------------------------------
   // Return the scope of the pointer to member type
   if (* this) return fTypeName->ToTypeBase()->PointerToMemberScope();
   return Dummy::Scope();
}


//-------------------------------------------------------------------------------
Reflex::PropertyList
Reflex::Type::Properties() const {
//-------------------------------------------------------------------------------
   if (* this) return fTypeName->ToTypeBase()->Properties();
   return Dummy::PropertyList();
}


//-------------------------------------------------------------------------------
Reflex::Type
Reflex::Type::RawType() const {
//-------------------------------------------------------------------------------
   if (* this) return fTypeName->ToTypeBase()->RawType();
   return Dummy::Type();
}


//-------------------------------------------------------------------------------
Reflex::Type
Reflex::Type::ReturnType() const {
//-------------------------------------------------------------------------------
   if (* this) return fTypeName->ToTypeBase()->ReturnType();
   return Dummy::Type();
}


//-------------------------------------------------------------------------------
void
Reflex::Type::SetSize(size_t s) const {
//-------------------------------------------------------------------------------
   if (* this) fTypeName->ToTypeBase()->SetSize(s);
}


//-------------------------------------------------------------------------------
void
Reflex::Type::SetTypeInfo(const std::type_info & ti) const {
//-------------------------------------------------------------------------------
   if (* this) fTypeName->ToTypeBase()->SetTypeInfo(ti);
}

//-------------------------------------------------------------------------------
size_t
Reflex::Type::SizeOf() const {
//-------------------------------------------------------------------------------
   if (* this) return fTypeName->ToTypeBase()->SizeOf();
   return 0;
}


//-------------------------------------------------------------------------------
const Reflex::OrderedContainer<Reflex::TemplateArgument>
Reflex::Type::TemplateArguments() const {
//-------------------------------------------------------------------------------
   if (* this) return fTypeName->ToTypeBase()->TemplateArguments();
   return Dummy::EmptyContainer();
}


//-------------------------------------------------------------------------------
Reflex::TypeTemplate
Reflex::Type::TemplateFamily() const {
//-------------------------------------------------------------------------------
   if (* this) return fTypeName->ToTypeBase()->TemplateFamily();
   return Dummy::TypeTemplate();
}


//-------------------------------------------------------------------------------
Reflex::Type
Reflex::Type::ToType() const {
//-------------------------------------------------------------------------------
   if (* this) return fTypeName->ToTypeBase()->ToType();
   return Dummy::Type();
}


//-------------------------------------------------------------------------------
const Reflex::Internal::TypeBase *
Reflex::Type::ToTypeBase() const {
//-------------------------------------------------------------------------------
   if (* this) return fTypeName->ToTypeBase();
   return 0;
}


//-------------------------------------------------------------------------------
const std::type_info &
Reflex::Type::TypeInfo() const {
//-------------------------------------------------------------------------------
   if (* this) return fTypeName->ToTypeBase()->TypeInfo(); 
   return typeid(void);
}


//-------------------------------------------------------------------------------
Reflex::ETYPE
Reflex::Type::TypeType() const {
//-------------------------------------------------------------------------------
   if (* this) return fTypeName->ToTypeBase()->TypeType();
   return kETUnresolved;
}


//-------------------------------------------------------------------------------
std::string
Reflex::Type::TypeTypeAsString() const {
//-------------------------------------------------------------------------------
   if (* this) return fTypeName->ToTypeBase()->TypeTypeAsString(); 
   return "kETUnresolved";
}

//-------------------------------------------------------------------------------
const Reflex::Container<Reflex::Type>
Reflex::Type::Types() {
//-------------------------------------------------------------------------------
   // Return the collection of known types; forwards to the Catalog.
   return Catalog::Instance().Types();
}

//------------------------------------------------------------------------------
void
Reflex::Type::GenerateDict(DictionaryGenerator & generator) const {
//------------------------------------------------------------------------------
// Generate Dictionary information about itself.
   if (* this) fTypeName->ToTypeBase()->GenerateDict(generator);
}


//-------------------------------------------------------------------------------
void
Reflex::Type::Unload() const {
//-------------------------------------------------------------------------------
//  Unload a type, i.e. delete the Internal::TypeName's TypeBase object.
   if (* this) delete fTypeName->ToTypeBase();
}

#ifdef REFLEX_CINT_MERGE
bool
Reflex::Type::operator&&(const Scope &right) const
{ return operator bool() && (bool)right; }
bool
Reflex::Type::operator&&(const Type &right) const 
{ return operator bool() && (bool)right; }
bool
Reflex::Type::operator&&(const Member &right) const 
{ return operator bool() && (bool)right; }
bool
Reflex::Type::operator||(const Scope &right) const 
{ return operator bool() && (bool)right; }
bool
Reflex::Type::operator||(const Type &right) const 
{ return operator bool() && (bool)right; }
bool
Reflex::Type::operator||(const Member &right) const 
{ return operator bool() || (bool)right; }
#endif
