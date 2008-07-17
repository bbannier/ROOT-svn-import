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

#include "Reflex/internal/TypeBase.h"

#include "Reflex/Type.h"
#include "Reflex/internal/OwnedPropertyList.h"
#include "Reflex/Object.h"
#include "Reflex/Scope.h"
#include "Reflex/internal/TypeName.h"
#include "Reflex/Base.h"
#include "Reflex/TypeTemplate.h"
#include "Reflex/DictionaryGenerator.h"


#include "Array.h"
#include "Pointer.h"
#include "PointerToMember.h"
#include "Union.h"
#include "Enum.h"
#include "Fundamental.h"
#include "Function.h"
#include "Class.h"
#include "Typedef.h"
#include "ClassTemplateInstance.h"
#include "FunctionMemberTemplateInstance.h"

#include "Reflex/Tools.h"

#include "Reflex/Builder/TypeBuilder.h"

//-------------------------------------------------------------------------------
Reflex::Internal::TypeBase::TypeBase( const char * nam, 
                                  size_t size,
                                  TYPE typeTyp, 
                                  const std::type_info & ti,
                                  const Type & finalType) 
   : fTypeInfo( &ti ), 
     fScope( Scope::__NIRVANA__() ),
     fSize( size ),
     fTypeType( typeTyp ),
     fPropertyList( OwnedPropertyList( new PropertyListImpl())),
     fBasePosition(Tools::GetBasePosition( nam)),
     fFinalType(finalType.Id() ? new Type(finalType) : 0 ),
     fRawType(0) {
//-------------------------------------------------------------------------------
// Construct the dictinary info for a type.
   Type t = TypeName::ByName( nam );
   if ( t.Id() == 0 ) { 
      fTypeName = new TypeName( nam, this, &ti ); 
   }
   else {
      fTypeName = (TypeName*)t.Id();
      if ( t.Id() != TypeName::ByTypeInfo(ti).Id()) fTypeName->SetTypeId( ti );
      if ( fTypeName->fTypeBase ) delete fTypeName->fTypeBase;
      fTypeName->fTypeBase = this;
   }

   if ( typeTyp != FUNDAMENTAL && 
        typeTyp != FUNCTION &&
        typeTyp != POINTER  ) {
      std::string sname = Tools::GetScopeName(nam);
      fScope = Scope::ByName(sname);
      if ( fScope.Id() == 0 ) fScope = (new ScopeName(sname.c_str(), 0))->ThisScope();
    
      // Set declaring At
      if ( fScope ) fScope.AddSubType(ThisType());
   }
}


//-------------------------------------------------------------------------------
Reflex::Internal::TypeBase::~TypeBase( ) {
//-------------------------------------------------------------------------------
// Destructor.
   fPropertyList.Delete();
   if ( fFinalType ) delete fFinalType;
   if ( fRawType ) delete fRawType;
   if ( fTypeName->fTypeBase == this ) fTypeName->fTypeBase = 0;
}


//-------------------------------------------------------------------------------
Reflex::Internal::TypeBase::operator Reflex::Scope () const {
//-------------------------------------------------------------------------------
// Conversion operator to Scope.
   switch ( fTypeType ) {
   case CLASS:
   case STRUCT:
   case TYPETEMPLATEINSTANCE:
   case UNION:
   case ENUM:
      return (dynamic_cast<const ScopeBase*>(this))->ThisScope();
   case TYPEDEF:
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
   return malloc( fSize );
}


//-------------------------------------------------------------------------------
void
Reflex::Internal::TypeBase::Deallocate( void * instance ) const {
//-------------------------------------------------------------------------------
// Deallocate the memory for this type from instance.
   free( instance );
}


//-------------------------------------------------------------------------------
Reflex::Object
Reflex::Internal::TypeBase::CastObject( const Type & /* to */,
                                                         const Object & /* obj */ ) const {
//-------------------------------------------------------------------------------
// Cast this type into "to" using object "obj"
   throw RuntimeError("This function can only be called on Class/Struct");
   return Dummy::Object();
}


//-------------------------------------------------------------------------------
Reflex::Object
Reflex::Internal::TypeBase::Construct( const Type &  /*signature*/,
                                   const std::vector < void * > & /*values*/, 
                                   void * /*mem*/ ) const {
//-------------------------------------------------------------------------------
// Construct this type.
   return Object(ThisType(), Allocate());
}


//-------------------------------------------------------------------------------
Reflex::Type
Reflex::Internal::TypeBase::DynamicType( const Object & /* obj */ ) const {
//-------------------------------------------------------------------------------
// Return the dynamic type info of this type.
   throw RuntimeError("Type::DynamicType can only be called on Class/Struct");
   return Dummy::Type();
}


//-------------------------------------------------------------------------------
Reflex::Type
Reflex::Internal::TypeBase::FinalType() const {
//-------------------------------------------------------------------------------
// Return the type without typedefs information.
   if ( fFinalType ) return *fFinalType;

   fFinalType = new Type(DetermineFinalType(ThisType()));
   return *fFinalType;
}


//-------------------------------------------------------------------------------
Reflex::Type
Reflex::Internal::TypeBase::DetermineFinalType(const Type& t) const {
//-------------------------------------------------------------------------------
// Return the type t without typedefs information.
   
   Type retType(t);

   switch ( t.TypeType()) {
   case TYPEDEF:
      retType = t.ToType().FinalType();
      break;
   case POINTER:
      retType = PointerBuilder(t.ToType().FinalType(), t.TypeInfo());
      break;
   case POINTERTOMEMBER:
      retType = PointerToMemberBuilder(t.ToType().FinalType(), t.PointerToMemberScope(), t.TypeInfo());
      break;
   case ARRAY:
      retType = ArrayBuilder(t.ToType().FinalType(), t.ArrayLength(), t.TypeInfo());
      break;
   case FUNCTION:
      {
         std::vector<Type> vecParFinal(t.FunctionParameterSize());
         for (size_t iPar = 0; iPar < t.FunctionParameterSize(); ++iPar)
            vecParFinal[iPar] = t.FunctionParameterAt(iPar).FinalType();
         retType = FunctionTypeBuilder(t.ReturnType().FinalType(), vecParFinal, t.TypeInfo());
         break;
      }
   case UNRESOLVED:
      return Dummy::Type();
   default:
      return t;
   }

   // copy fModifiers
   if (t.Is(gCONST))
      retType = ConstBuilder(retType);
   if (t.Is(gREFERENCE))
      retType = ReferenceBuilder(retType);
   if (t.Is(gVOLATILE))
      retType = VolatileBuilder(retType);

   return retType;
}


//-------------------------------------------------------------------------------
void
Reflex::Internal::TypeBase::HideName() const {
//-------------------------------------------------------------------------------
// Append the string " @HIDDEN@" to a type name.
   fTypeName->HideName();
}


//-------------------------------------------------------------------------------
const std::string&
Reflex::Internal::TypeBase::Name(std::string& buf, unsigned int mod ) const {
//-------------------------------------------------------------------------------
// Return the name of the type, using buffer buf when calculating it
   if (mod & SCOPED)
      return (buf += fTypeName->Name());
   return (buf += std::string(fTypeName->Name(), fBasePosition));
}


//-------------------------------------------------------------------------------
const std::string &
Reflex::Internal::TypeBase::SimpleName( size_t & pos, 
                                        unsigned int mod ) const {
//-------------------------------------------------------------------------------
// Return the name of the type.
   if (mod & SCOPED) {
      pos = 0;
      return fTypeName->Name();
   }
   pos = fBasePosition;
   return fTypeName->Name();
}


//-------------------------------------------------------------------------------
Reflex::Type
Reflex::Internal::TypeBase::RawType() const {
//-------------------------------------------------------------------------------
// Return the raw type of this type, removing all info of pointers, arrays, typedefs.
   if ( fRawType ) return *fRawType;
   
   Type rawType = ThisType();
   
   while ( true ) {
      
      switch (rawType.TypeType()) {
         
      case POINTER:
      case POINTERTOMEMBER:
      case TYPEDEF:
      case ARRAY:
         rawType = rawType.ToType();
         break;
      case UNRESOLVED:
         return Dummy::Type();
      default:
         fRawType = new Type(rawType);
         return *fRawType;
      }     
   }
}


//-------------------------------------------------------------------------------
Reflex::Type
Reflex::Internal::TypeBase::TemplateArgumentAt( size_t /* nth */ ) const {
//-------------------------------------------------------------------------------
// Return the nth template argument.
   return Dummy::Type();
}


//-------------------------------------------------------------------------------
Reflex::Type
Reflex::Internal::TypeBase::ToType() const {
//-------------------------------------------------------------------------------
// Return the underlying type.
   return Dummy::Type();
}


//-------------------------------------------------------------------------------
Reflex::Type
Reflex::Internal::TypeBase::ThisType() const {
//-------------------------------------------------------------------------------
// Return the Type object pointing to this TypeBase.
   return fTypeName->ThisType();
}

