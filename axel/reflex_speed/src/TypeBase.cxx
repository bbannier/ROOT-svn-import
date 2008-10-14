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

#include "TypeBase.h"

#include "Reflex/Catalog.h"
#include "Reflex/Object.h"

#include "Reflex/DictionaryGenerator.h"
#include "Reflex/Tools.h"

#include "Reflex/Builder/TypeBuilder.h"

#include "TypeName.h"
#include "ScopeBase.h"
#include "ScopeName.h"
#include "PropertyListImpl.h"
#include "OwnedPropertyList.h"

//-------------------------------------------------------------------------------
Reflex::Internal::TypeBase::TypeBase(const char * nam, 
                                     unsigned int modifiers,
                                     size_t size,
                                     ETYPE typeTyp, 
                                     const std::type_info & ti,
                                     const Catalog& catalog,
                                     const Type & finalType)
   : fTypeInfo(&ti),
     fTypeModifiers(modifiers),
     fSize(size),
     fTypeType(typeTyp),
     fPropertyList(OwnedPropertyList(new PropertyListImpl())),
     fBasePosition(Tools::GetBasePosition(nam)),
     fFinalType(finalType.Id() ? new Type(finalType) : 0),
     fRawType(0) {
//-------------------------------------------------------------------------------
// Construct the dictinary info for a type.
   Type t = catalog.ByName(nam);
   if (t.Id() == 0) { 
      fTypeName = new TypeName(nam, this, catalog, &ti); 
   }
   else {
      fTypeName = (TypeName*)t.Id();
      if (t.Id() != catalog.ByTypeInfo(ti).Id())
         fTypeName->SetTypeId(ti);
      if (fTypeName->ToTypeBase())
         fTypeName->UpdateTypeBase(this);
   }

   if (typeTyp != kETFundamental && 
        typeTyp != kETFunction &&
        typeTyp != kETPointer ) {
      Scope declScope = fTypeName->DeclaringScope();
      // Add to declaring scope
      if (declScope)
         declScope.AddSubType(ThisType());
   }
}


//-------------------------------------------------------------------------------
Reflex::Internal::TypeBase::~TypeBase() {
//-------------------------------------------------------------------------------
// Destructor.
   fPropertyList.Delete();
   delete fFinalType;
   delete fRawType;
   if (fTypeName->ToTypeBase() == this)
      fTypeName->UpdateTypeBase(0);
}


//-------------------------------------------------------------------------------
Reflex::Internal::TypeBase::operator Reflex::Scope () const {
//-------------------------------------------------------------------------------
// Conversion operator to Scope.
   switch (fTypeType) {
   case kETClass:
   case kETStruct:
   case kETTypeTemplateInstance:
   case kETUnion:
   case kETEnum:
      return (dynamic_cast<const ScopeBase*>(this))->ThisScope();
   case kETTypedef:
      return FinalType();
   default:
      return Dummy::Scope();
   }
}


//-------------------------------------------------------------------------------
void *
Reflex::Internal::TypeBase::Allocate() const {
//-------------------------------------------------------------------------------
// Allocate memory for this type.
   return malloc(fSize);
}


//-------------------------------------------------------------------------------
void
Reflex::Internal::TypeBase::Deallocate(void * instance) const {
//-------------------------------------------------------------------------------
// Deallocate the memory for this type from instance.
   free(instance);
}


//-------------------------------------------------------------------------------
Reflex::Object
Reflex::Internal::TypeBase::CastObject(const Type & /* to */,
                                                         const Object & /* obj */) const {
//-------------------------------------------------------------------------------
// Cast this type into "to" using object "obj"
   throw RuntimeError("This function can only be called on Class/Struct");
   return Dummy::Object();
}


//-------------------------------------------------------------------------------
Reflex::Object
Reflex::Internal::TypeBase::Construct(const Type &  /*signature*/,
                                   const std::vector < void * > & /*values*/, 
                                   void * /*mem*/) const {
//-------------------------------------------------------------------------------
// Construct this type.
   return Object(ThisType(), Allocate());
}


//-------------------------------------------------------------------------------
Reflex::Type
Reflex::Internal::TypeBase::DetermineFinalType(const Type& t) const {
//-------------------------------------------------------------------------------
// Return the type t without typedefs information.
   
   Type retType(t);

   switch (t.TypeType()) {
   case kETTypedef:
      retType = t.ToType().FinalType();
      break;
   case kETPointer:
      retType = PointerBuilder(t.ToType().FinalType(), t.TypeInfo(), InCatalog());
      break;
   case kETPointerToMember:
      retType = PointerToMemberBuilder(t.ToType().FinalType(), t.PointerToMemberScope(), t.TypeInfo(), InCatalog());
      break;
   case kETArray:
      retType = ArrayBuilder(t.ToType().FinalType(), t.ArrayLength(), t.TypeInfo(), InCatalog());
      break;
   case kETFunction:
      {
         std::vector<Type> vecParFinal(t.FunctionParameters().Size());
         size_t idx = 0;
         Type_Iterator iParEnd = t.FunctionParameters().End();
         for (Type_Iterator iPar = t.FunctionParameters().Begin(); iPar != iParEnd; ++iPar, ++idx)
            vecParFinal[idx] = iPar->FinalType();
         retType = FunctionTypeBuilder(t.ReturnType().FinalType(), vecParFinal, t.TypeInfo(), InCatalog());
         break;
      }
   case kETUnresolved:
      return Dummy::Type();
   default:
      return t;
   }

   // copy fModifiers
   if (t.Is(kConst))
      retType = ConstBuilder(retType);
   if (t.Is(kReference))
      retType = ReferenceBuilder(retType);
   if (t.Is(kVolatile))
      retType = VolatileBuilder(retType);

   return retType;
}


//-------------------------------------------------------------------------------
Reflex::Type
Reflex::Internal::TypeBase::FinalType() const {
//-------------------------------------------------------------------------------
// Return the type without typedefs information.
   if (fFinalType) return *fFinalType;

   fFinalType = new Type(DetermineFinalType(ThisType()));
   return *fFinalType;
}


//-------------------------------------------------------------------------------
void
Reflex::Internal::TypeBase::HideName() const {
//-------------------------------------------------------------------------------
// Append the string " @HIDDEN@" to a type name.
   fTypeName->HideName();
}


//-------------------------------------------------------------------------------
const Reflex::Catalog&
Reflex::Internal::TypeBase::InCatalog() const {
//-------------------------------------------------------------------------------
// Retrieve the Catalog containing the type.
   return fTypeName->InCatalog();
}


//-------------------------------------------------------------------------------
const std::string&
Reflex::Internal::TypeBase::Name(std::string& buf, unsigned int mod) const {
//-------------------------------------------------------------------------------
// Return the name of the type, using buffer buf when calculating it
   if (mod & kScoped)
      return (buf += fTypeName->Name());
   return (buf += std::string(fTypeName->Name(), fBasePosition));
}


//-------------------------------------------------------------------------------
Reflex::Type
Reflex::Internal::TypeBase::RawType() const {
//-------------------------------------------------------------------------------
// Return the raw type of this type, removing all info of pointers, arrays, typedefs.
   if (fRawType) return *fRawType;
   
   Type rawType = ThisType();
   
   while (true) {
      
      switch (rawType.TypeType()) {
         
      case kETPointer:
      case kETPointerToMember:
      case kETTypedef:
      case kETArray:
         rawType = rawType.ToType();
         break;
      case kETUnresolved:
         return Dummy::Type();
      default:
         fRawType = new Type(rawType);
         return *fRawType;
      }     
   }
}


//-------------------------------------------------------------------------------
const std::string &
Reflex::Internal::TypeBase::SimpleName(size_t & pos, 
                                       unsigned int mod) const {
//-------------------------------------------------------------------------------
// Return the name of the type.
   if (mod & kScoped)
      pos = 0;
   else
      pos = fBasePosition;
   return fTypeName->Name();
}


//-------------------------------------------------------------------------------
Reflex::Type
Reflex::Internal::TypeBase::ThisType() const {
//-------------------------------------------------------------------------------
// Return the Type object pointing to this TypeBase.
   return fTypeName->ThisType();
}

