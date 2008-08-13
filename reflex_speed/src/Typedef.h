// @(#)root/reflex:$Id$
// Author: Stefan Roiser 2004

// Copyright CERN, CH-1211 Geneva 23, 2004-2006, All rights reserved.
//
// Permission to use, copy, modify, and distribute this software for any
// purpose is hereby granted without fee, provided that this copyright and
// permissions notice appear in all copies and derivatives.
//
// This software is provided "as is" without express or implied warranty.

#ifndef Reflex_Typedef
#define Reflex_Typedef

// Include files
#include "TypeBase.h"
#include "Reflex/Type.h"
#include "Reflex/EntityProperty.h"

namespace Reflex {
   // forward declarations
   class Base;
   class Catalog;
   class Member;
   class Scope;
   class Object;
   class MemberTemplate;
   class TypeTemplate;

namespace Internal {

   /**
   * @class Typedef Typedef.h Reflex/Typedef.h
   * @author Stefan Roiser
   * @date 24/11/2003
   * @ingroup Ref
   */
   class Typedef : public TypeBase {

   public:

      /** constructor */
      Typedef(const char * typ,
         const Type & typedefType,
         const Catalog& catalog,
         ETYPE typeTyp = kTypedef,
         unsigned int modifiers = 0,
         const Type & finalType = Dummy::Type()) ;


      /** destructor */
      virtual ~Typedef() {}


      /**
      * CastObject an object from this class At to another one
      * @param  to is the class At to cast into
      * @param  obj the memory AddressGet of the object to be casted
      */
      virtual Object CastObject(const Type & to, 
         const Object & obj) const;


      /**
      * TypeInfo will return the c++ type_info object of the At
      * @return type_info object of At
      */
      virtual const std::type_info & TypeInfo() const;


      /**
      * typedefType will return a pointer to the At of the typedef.
      * @return pointer to Type of MemberAt et. al.
      */
      virtual Type ToType() const;

   private:  

      bool ForwardStruct() const;
      bool ForwardTemplate() const;
      bool ForwardFunction() const;

   private:

      /**
      * pointer to the type of the typedef
      * @label typedef type
      * @link aggregation
      * @supplierCardinality 1
      * @clientCardinality 1
      */
      Type fTypedefType;

   }; // class Typedef
} //namespace Internal
} //namespace Reflex

#include "Reflex/Base.h"
#include "Reflex/Object.h"
#include "Reflex/Member.h"
#include "Reflex/MemberTemplate.h"
#include "Reflex/Scope.h"
#include "Reflex/TypeTemplate.h"

//-------------------------------------------------------------------------------
inline Reflex::Object
Reflex::Internal::Typedef::CastObject(const Type & to,
                                      const Object & obj) const {
//-------------------------------------------------------------------------------
   if (ForwardStruct()) return fTypedefType.CastObject(to, obj);
   return Dummy::Object();
}


//-------------------------------------------------------------------------------
inline const std::type_info & Reflex::Internal::Typedef::TypeInfo() const {
//-------------------------------------------------------------------------------
   if (*fTypeInfo != typeid(UnknownType)) return *fTypeInfo;
   Type current = ThisType();
   while (current.TypeType() == kTypedef) current = current.ToType();
   if (current && current.TypeInfo() != typeid(UnknownType)) fTypeInfo = &current.TypeInfo();
   return *fTypeInfo;
}


//-------------------------------------------------------------------------------
inline Reflex::Type Reflex::Internal::Typedef::ToType() const {
//-------------------------------------------------------------------------------
   return fTypedefType;
}


//-------------------------------------------------------------------------------
inline bool Reflex::Internal::Typedef::ForwardStruct() const {
//-------------------------------------------------------------------------------
   switch (fTypedefType.TypeType()) {
   case kTypedef:
   case kClass:
   case kStruct:
   case kTypeTemplateInstance:
      return true;
   default:
      return false;
   }
}


//-------------------------------------------------------------------------------
inline bool Reflex::Internal::Typedef::ForwardTemplate() const {
//-------------------------------------------------------------------------------
   switch (fTypedefType.TypeType()) {
   case kTypedef:
   case kTypeTemplateInstance:
   case kMemberTemplateInstance:
      return true;
   default:
      return false;
   }
}


//-------------------------------------------------------------------------------
inline bool Reflex::Internal::Typedef::ForwardFunction() const {
//-------------------------------------------------------------------------------
   switch (fTypedefType.TypeType()) {
   case kTypedef:
   case kFunction:
      return true;
   default:
      return false;
   }
}


 
#endif // Reflex_Typedef



