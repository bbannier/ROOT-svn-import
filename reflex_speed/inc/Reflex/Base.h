// @(#)root/reflex:$Id$
// Author: Stefan Roiser 2004

// Copyright CERN, CH-1211 Geneva 23, 2004-2006, All rights reserved.
//
// Permission to use, copy, modify, and distribute this software for any
// purpose is hereby granted without fee, provided that this copyright and
// permissions notice appear in all copies and derivatives.
//
// This software is provided "as is" without express or implied warranty.

#ifndef REFLEX_BASE_H
#define REFLEX_BASE_H 1

// Include files
#include "Reflex/Kernel.h"
#include "Reflex/Type.h"
#include "Reflex/Scope.h"
#include "Reflex/EntityProperty.h"

namespace Reflex {

   namespace Internal {
      // forward declarations
      class Class;
   }

   /**
   * @class Base Base.h Reflex/Base.h
   * @author Stefan Roiser
   * @date   2004-01-28
   * @ingroup Ref
   */
   class RFLX_API Base {

      friend class Internal::Class;

   public:

      /** default constructor */
      Base();


      /** constructor */
      Base(const Type &   baseType,
         OffsetFunction offsetFP,
         unsigned int   modifiers = 0);


      /** destructor */
      virtual ~Base() {}


      /**
      * the bool operator returns true if the type of the base is resolved (implemented)
      * @return true if base type is implemented 
      */
      operator bool () const;


      /**
      * Name will return the string represenation of the base class
      * @param  typedefexp expand typedefs or not
      * @return string represenation of base class
      */
      std::string Name(unsigned int mod = 0) const;


      /**
      * Check whether the entity property is set for the base. You can
      * combine checks, e.g. t.Is(kClass && kPublic) would check whether
      * this is a public base class.
      * Only public / protected / private and virtual are check with the base;
      * the ETYPE-like properties (class / struct / ...) are forwarded to
      * the base's type.
      * @param descr the entity property to check for; see EntityProperty.
      * @return whether descr is set.
      */
      bool Is(const EntityProperty& prop) const;


      /**
      * Offset will return the Offset to the base class as int
      * @return Offset to base class
      */
      size_t Offset(void * mem = 0) const;


      /** 
      * OffsetFP will return a pointer to the function which calculates the Offset
      * between the two classes
      * @return pointer to Offset calculating function
      */
      OffsetFunction OffsetFP() const;


      /**
      * ToType will return this base classes type
      * @param mod accepts kFinal to go to the final type for a typedef
      * @return type of base class
      */
      Type ToType() const;


      /** 
      * ToScope will return this base classes scope
      * @return this base class as scope
      */
      Scope ToScope() const;

   private:

      const Internal::Class * BaseClass() const;

   private:

      /** function pointer to Stubfunction for Offset calculation */
      OffsetFunction fOffsetFP;


      /** modifiers of inheritance relation */
      unsigned int fModifiers;


      /**
      * pointer to base class
      * @label base bype
      * @link aggregation
      * @supplierCardinality 1
      * @clientCardinality 1
      */
      Type fBaseType;


      /** 
      * back link to the class corresponding to the base
      * @label base class
      * @link aggregation
      * @clientCardinality 1
      * @supplierCardinality 0..1
      **/
      mutable
         const Internal::Class * fBaseClass;

   }; // class Base
} // namespace Reflex

 
//-------------------------------------------------------------------------------
inline Reflex::Base::Base() 
//-------------------------------------------------------------------------------
   : fOffsetFP(0),
     fModifiers(0),
     fBaseType(0, 0),
     fBaseClass(0) {}



//-------------------------------------------------------------------------------
inline Reflex::Base::operator bool () const {
//-------------------------------------------------------------------------------
   if (fBaseType) return true;
   return false;
}


//-------------------------------------------------------------------------------
inline bool Reflex::Base::Is(const EntityProperty& prop) const {
//-------------------------------------------------------------------------------
   return prop.Eval(fModifiers, ToType().TypeType());
}


//-------------------------------------------------------------------------------
inline size_t Reflex::Base::Offset(void * mem) const {
//-------------------------------------------------------------------------------
   return fOffsetFP(mem);
}


//-------------------------------------------------------------------------------
inline Reflex::OffsetFunction Reflex::Base::OffsetFP() const {
//-------------------------------------------------------------------------------
   return fOffsetFP;
}


//-------------------------------------------------------------------------------
inline Reflex::Type Reflex::Base::ToType() const {
//-------------------------------------------------------------------------------
   return fBaseType;
}


//-------------------------------------------------------------------------------
inline Reflex::Scope Reflex::Base::ToScope() const {
//-------------------------------------------------------------------------------
   // We are invoking "Type::operator Scope() const" here,
   // be very careful with the cast (do not cast to a reference).
   return static_cast<const Scope>(fBaseType);
}

#endif // Reflex_Base
