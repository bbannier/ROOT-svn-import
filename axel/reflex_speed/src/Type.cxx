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

#include "Reflex/Container.h"
#include "Reflex/Object.h"
#include "Reflex/Scope.h"
#include "Reflex/Base.h"
#include "Reflex/MemberTemplate.h"
#include "Reflex/TypeTemplate.h"
#include "Reflex/PropertyList.h"
#include "Reflex/DictionaryGenerator.h"

#include "Reflex/internal/TypeName.h"
#include "Reflex/internal/TypeBase.h"


#include "Enum.h"
#include "Union.h"
#include "Class.h"
#include "Reflex/Tools.h"


//-------------------------------------------------------------------------------
Reflex::Type::operator bool () const {
//-------------------------------------------------------------------------------
   if ( this->fTypeName && this->fTypeName->fTypeBase ) return true;
   return false;
}


//-------------------------------------------------------------------------------
Reflex::Type::operator Reflex::Scope () const {
//-------------------------------------------------------------------------------
// Conversion operator to Scope.
   if ( * this ) return *(fTypeName->fTypeBase);
   return Dummy::Scope();
}


//-------------------------------------------------------------------------------
void *
Reflex::Type::Allocate() const {
//-------------------------------------------------------------------------------
   if ( * this ) return fTypeName->fTypeBase->Allocate();
   return 0;
}


//-------------------------------------------------------------------------------
size_t
Reflex::Type::ArrayLength() const {
//-------------------------------------------------------------------------------
   if ( * this ) return fTypeName->fTypeBase->ArrayLength();
   return 0;
}


//-------------------------------------------------------------------------------
Reflex::Object
Reflex::Type::CastObject( const Type & to,
                          const Object & obj ) const {
//-------------------------------------------------------------------------------
// Cast the current type to "to" using the object obj.
   if ( * this ) return fTypeName->fTypeBase->CastObject( to, obj );
   return Dummy::Object();
}


//-------------------------------------------------------------------------------
Reflex::Object
Reflex::Type::Construct( const Type & signature,
                         const std::vector < void * > & values, 
                         void * mem ) const {
//-------------------------------------------------------------------------------
// Construct this type and return it as an object. Signature can be used for overloaded
// constructors. Values is a collection of memory addresses of paramters. Mem the memory
// address for in place construction.
   if ( * this ) return fTypeName->fTypeBase->Construct( signature, 
                                                         values, 
                                                         mem ); 
   return Dummy::Object();
}


//-------------------------------------------------------------------------------
void
Reflex::Type::Deallocate( void * instance ) const {
//-------------------------------------------------------------------------------
   if ( * this ) fTypeName->fTypeBase->Deallocate( instance ); 
}


//-------------------------------------------------------------------------------
Reflex::Scope
Reflex::Type::DeclaringScope() const {
//-------------------------------------------------------------------------------
   if ( * this ) return fTypeName->fTypeBase->DeclaringScope();
   return Dummy::Scope();
}


//-------------------------------------------------------------------------------
void
Reflex::Type::Destruct( void * instance, 
                        bool dealloc ) const {
//-------------------------------------------------------------------------------
   if ( * this ) fTypeName->fTypeBase->Destruct( instance, dealloc ); 
}


//-------------------------------------------------------------------------------
Reflex::Type
Reflex::Type::DynamicType( const Object & obj ) const {
//-------------------------------------------------------------------------------
// Return the dynamic type of this type.
   if ( * this ) return fTypeName->fTypeBase->DynamicType( obj );
   return Dummy::Type();
}


//-------------------------------------------------------------------------------
Reflex::Type
Reflex::Type::FinalType() const {
//-------------------------------------------------------------------------------
   if ( * this ) return Reflex::Type(fTypeName->fTypeBase->FinalType(), fModifiers, APPEND);
   return *this;
}


//-------------------------------------------------------------------------------
bool
Reflex::Type::IsComplete() const {
//-------------------------------------------------------------------------------
   if ( * this ) return fTypeName->fTypeBase->IsComplete();
   return false;
}


//-------------------------------------------------------------------------------
bool
Reflex::Type::DetermineEquivalence( const Type & typ, unsigned int modifiers_mask, bool alsoReturn ) const {
//-------------------------------------------------------------------------------
// Check if two types are equivalent. It will compare the information of the type
// depending on the TypeType. If the type is a fuction and alsoReturn is set,
// the functions' return types are compared, too.

   if ( *this == typ ) return true;

   Type t1 = *this;
   Type t2 = typ;

   unsigned int mod1 = t1.fModifiers | modifiers_mask;
   unsigned int mod2 = t2.fModifiers | modifiers_mask;

   while (t1.Is(gTYPEDEF)) { 
      t1 = t1.ToType();
      mod1 |= t1.fModifiers;
   }
   while ( t2.Is(gTYPEDEF)) {
      t2 = t2.ToType();
      mod2 |= t2.fModifiers;
   }

   if (mod1 != mod2)
      return false;
   if (t1.TypeType() != t2.TypeType())
      return false;

   if (t1.Is(gARRAY) && t1.ArrayLength() != t2.ArrayLength())
      return false;

   static EntityProperty needToType(gPOINTER || gPOINTERTOMEMBER || gARRAY);

   if (t1.Is(needToType))
      // set alsoReturn to true because we know it's not a function and void(*)() != int(*)()
      return t1.ToType().DetermineEquivalence(t2.ToType(),modifiers_mask, true);

   if (!t1.Is(gFUNCTION))
      return t1.fTypeName == t2.fTypeName;
   else {
      if (alsoReturn)
         if (!t1.ReturnType().IsEquivalentTo(t2.ReturnType(),modifiers_mask))
            return false;

      if (t1.FunctionParameters().Size() != t2.FunctionParameters().Size())
         return false;

      Type_Iterator pi1;
      Type_Iterator pi2;
      for ( pi1 = t1.FunctionParameters().Begin(), pi2 = t2.FunctionParameters().Begin(); 
            pi1 != t1.FunctionParameters().End(),  pi2 != t2.FunctionParameters().End(); 
            ++pi1, ++pi2 ) {

         // set alsoReturn to true because we know it's not a function and void(*)() != int(*)()
         if (!pi1->DetermineEquivalence(*pi2, modifiers_mask, true))
            return false;
      }
      return true;
   }
   return false;
}


//-------------------------------------------------------------------------------
std::string
Reflex::Type::Name( unsigned int mod ) const {
//-------------------------------------------------------------------------------
// Return the name of the type (qualified and scoped if requested)
   std::string buf;
   return Name(buf, mod);
}


//-------------------------------------------------------------------------------
const std::string&
Reflex::Type::Name( std::string& buf, unsigned int mod ) const {
//-------------------------------------------------------------------------------
// Return the name of the type (qualified and scoped if requested)

   static const std::string sCVcv = "const volatile";
   static const std::string sCVc  = "const";
   static const std::string sCVv  = "volatile";


   if ( (!(mod & QUALIFIED) || !fModifiers) && (*this) ) { 
      // most common case
      return fTypeName->fTypeBase->Name(buf, mod);
   }

   const std::string *cv = 0;

   /** apply qualifications if wanted */
   if (mod & QUALIFIED) {
      if (Is(gCONST && gVOLATILE)) cv = &sCVcv;
      else if (Is(gCONST))         cv = &sCVc;
      else if (Is(gVOLATILE))      cv = &sCVv;
   }

   bool isPtrOrFunc = Is(gPOINTER || gFUNCTION);

   /** if At is not a pointer qualifiers can be put before */
   if (cv && !isPtrOrFunc)
      buf += *cv + " ";
  
   /** use implemented names if available */
   if (*this)
      fTypeName->fTypeBase->Name(buf, mod );
   /** otherwise use the Internal::TypeName */
   else {
      if ( fTypeName ) {
         /** unscoped At Name */
         if ( mod & SCOPED )
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
   if ((mod & QUALIFIED) && Is(gREFERENCE))
      buf += "&";

   return buf;
}

//-------------------------------------------------------------------------------
const char *
Reflex::Type::Name_c_str() const {
//-------------------------------------------------------------------------------
   if ( fTypeName ) return fTypeName->Name_c_str();
   return "";
}


//-------------------------------------------------------------------------------
Reflex::Scope
Reflex::Type::PointerToMemberScope() const {
//-------------------------------------------------------------------------------
   // Return the scope of the pointer to member type
   if ( * this ) return fTypeName->fTypeBase->PointerToMemberScope();
   return Dummy::Scope();
}


//-------------------------------------------------------------------------------
Reflex::PropertyList
Reflex::Type::Properties() const {
//-------------------------------------------------------------------------------
   if ( * this ) return fTypeName->fTypeBase->Properties();
   return Dummy::PropertyList();
}


//-------------------------------------------------------------------------------
Reflex::Type
Reflex::Type::RawType() const {
//-------------------------------------------------------------------------------
   if ( * this ) return fTypeName->fTypeBase->RawType();
   return Dummy::Type();
}


//-------------------------------------------------------------------------------
Reflex::Type
Reflex::Type::ReturnType() const {
//-------------------------------------------------------------------------------
   if ( * this ) return fTypeName->fTypeBase->ReturnType();
   return Dummy::Type();
}


//-------------------------------------------------------------------------------
void
Reflex::Type::SetSize( size_t s ) const {
//-------------------------------------------------------------------------------
   if ( * this ) fTypeName->fTypeBase->SetSize( s );
}


//-------------------------------------------------------------------------------
void
Reflex::Type::SetTypeInfo( const std::type_info & ti ) const {
//-------------------------------------------------------------------------------
   if ( * this ) fTypeName->fTypeBase->SetTypeInfo( ti );
}

//-------------------------------------------------------------------------------
size_t
Reflex::Type::SizeOf() const {
//-------------------------------------------------------------------------------
   if ( * this ) return fTypeName->fTypeBase->SizeOf();
   return 0;
}


//-------------------------------------------------------------------------------
Reflex::TypeTemplate
Reflex::Type::TemplateFamily() const {
//-------------------------------------------------------------------------------
   if ( * this ) return fTypeName->fTypeBase->TemplateFamily();
   return Dummy::TypeTemplate();
}


//-------------------------------------------------------------------------------
Reflex::Type
Reflex::Type::ToType() const {
//-------------------------------------------------------------------------------
   if ( * this ) return fTypeName->fTypeBase->ToType();
   return Dummy::Type();
}


//-------------------------------------------------------------------------------
const Reflex::Internal::TypeBase *
Reflex::Type::ToTypeBase() const {
//-------------------------------------------------------------------------------
   if ( * this ) return fTypeName->fTypeBase;
   return 0;
}


//-------------------------------------------------------------------------------
const std::type_info &
Reflex::Type::TypeInfo() const {
//-------------------------------------------------------------------------------
   if ( * this ) return fTypeName->fTypeBase->TypeInfo(); 
   return typeid(void);
}


//-------------------------------------------------------------------------------
Reflex::TYPE
Reflex::Type::TypeType() const {
//-------------------------------------------------------------------------------
   if ( * this ) return fTypeName->fTypeBase->TypeType();
   return UNRESOLVED;
}


//-------------------------------------------------------------------------------
std::string
Reflex::Type::TypeTypeAsString() const {
//-------------------------------------------------------------------------------
   if ( * this ) return fTypeName->fTypeBase->TypeTypeAsString(); 
   return "UNRESOLVED";
}


//------------------------------------------------------------------------------
void
Reflex::Type::GenerateDict( DictionaryGenerator & generator) const {
//------------------------------------------------------------------------------
// Generate Dictionary information about itself.
   if ( * this ) fTypeName->fTypeBase->GenerateDict( generator );
}


//-------------------------------------------------------------------------------
void
Reflex::Type::Unload() const {
//-------------------------------------------------------------------------------
//  Unload a type, i.e. delete the Internal::TypeName's TypeBase object.
   if ( * this ) delete fTypeName->fTypeBase;
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
