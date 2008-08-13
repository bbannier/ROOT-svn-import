// @(#)root/reflex:$Id$
// Author: Stefan Roiser 2004

// Copyright CERN, CH-1211 Geneva 23, 2004-2006, All rights reserved.
//
// Permission to use, copy, modify, and distribute this software for any
// purpose is hereby granted without fee, provided that this copyright and
// permissions notice appear in all copies and derivatives.
//
// This software is provided "as is" without express or implied warranty.

#ifndef Reflex_Array
#define Reflex_Array

// Include files
#include "TypeBase.h"
#include "Reflex/Type.h"

namespace Reflex {
   class Catalog;

namespace Internal {

   // forward declarations

   /**
   * @class Array Array.h Reflex/Array.h
   * @author Stefan Roiser
   * @date 24/11/2003
   * @ingroup Ref
   */
   class Array : public TypeBase {

   public:

      /** default constructor */
      Array(const Type & arrayType,
            unsigned int modifiers,
            size_t len,
            const std::type_info & typeinfo,
            const Catalog& catalog);


      /** destructor */
      virtual ~Array() {}


      /**
      * Name returns the name of the type
      * @param buf preallocated buffer to work on when calculating the name
      * @return name of type
      */
      virtual const std::string& Name(std::string& buf, unsigned int mod = kScoped | kQualified) const;


      /**
      * size returns the size of the array
      * @return size of array
      */
      size_t ArrayLength() const;


      /**
      * arrayType will return a pointer to the At of the array.
      * @return pointer to Type of MemberAt et. al.
      */
      Type ToType() const;


      /** static funtion that composes the At Name */
      static std::string BuildTypeNameSuffix(Type arraytype, size_t len);

      /** static funtion that composes the At Name */
      static std::string BuildTypeName(Type arraytype, size_t len) {
         return arraytype.FinalType().Name() + BuildTypeNameSuffix(arraytype, len);
      }

   private:

      /**
      * Type of the array
      * @label array type
      * @link aggregation
      * @supplierCardinality 1
      * @clientCardinality 1
      */
      Type fArrayType;


      /** the Length of the array */
      size_t fLength;

      /** trailing name string representing the array size */
      mutable
         std::string fNameArraySizeSuffix;

   }; // class Array
} //namespace Internal
} //namespace Reflex


//-------------------------------------------------------------------------------
inline size_t Reflex::Internal::Array::ArrayLength() const { 
//-------------------------------------------------------------------------------
   return fLength; 
}


//-------------------------------------------------------------------------------
inline Reflex::Type Reflex::Internal::Array::ToType() const {
//-------------------------------------------------------------------------------
   return fArrayType;
}

#endif // Reflex_Array
