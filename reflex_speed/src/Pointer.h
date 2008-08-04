// @(#)root/reflex:$Id$
// Author: Stefan Roiser 2004

// Copyright CERN, CH-1211 Geneva 23, 2004-2006, All rights reserved.
//
// Permission to use, copy, modify, and distribute this software for any
// purpose is hereby granted without fee, provided that this copyright and
// permissions notice appear in all copies and derivatives.
//
// This software is provided "as is" without express or implied warranty.

#ifndef Reflex_Pointer
#define Reflex_Pointer

// Include files
#include "TypeBase.h"
#include "Reflex/Type.h"

namespace Reflex {

namespace Internal {

   // forward declarations

   /**
   * @class Pointer Pointer.h Reflex/Pointer.h
   * @author Stefan Roiser
   * @date 24/11/2003
   * @ingroup Ref
   */
   class Pointer : public TypeBase {
   public:

      /** default constructor */
      Pointer(const Type & pointerType,
         unsigned int modifiers,
         const std::type_info & ti);

      /** destructor */
      virtual ~Pointer() {}


      /**
      * Name returns the name of the type
      * @param  buf buffer to be used for calculating name
      * @param  mod qualifiers can be or'ed 
      *   kFinal     - resolve typedefs
      *   kScoped    - fully scoped name 
      *   kQualified - cv, reference qualification 
      * @return name of the type
      */
      const std::string& Name(std::string& buf, unsigned int mod = kScoped | kQualified) const;


      /**
      * ToType will return a pointer to the type the pointer points to.
      * @return pointer to Type of MemberAt et. al.
      */
      Type ToType() const;


      /** static funtion that composes the typename */
      static const std::string& BuildTypeName(
         const Type & pointerType,
         unsigned int mod = kScoped | kQualified) {
         std::string buf;
         return BuildTypeName(buf, pointerType, mod);
      }

      /** static funtion that composes the typename */
      static const std::string& BuildTypeName(std::string& buf,
         const Type & pointerType,
         unsigned int mod = kScoped | kQualified);

   private:

      /**
      * The Type the Pointer points to
      * @label pointer type
      * @link aggregation
      * @supplierCardinality 1
      * @clientCardinality 1
      */
      Type fPointerType;

   }; // class Pointer
} //namespace Internal
} //namespace Reflex


//-------------------------------------------------------------------------------
inline Reflex::Type
Reflex::Internal::Pointer::ToType() const {
//-------------------------------------------------------------------------------
   return fPointerType;
}

//-------------------------------------------------------------------------------
inline const std::string&
Reflex::Internal::Pointer::Name(std::string& buf, unsigned int mod) const {
//-------------------------------------------------------------------------------
   return BuildTypeName(buf, fPointerType, mod);
}

#endif // Reflex_Pointer

