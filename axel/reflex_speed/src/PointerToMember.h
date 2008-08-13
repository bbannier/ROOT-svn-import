// @(#)root/reflex:$Id$
// Author: Stefan Roiser 2004

// Copyright CERN, CH-1211 Geneva 23, 2004-2006, All rights reserved.
//
// Permission to use, copy, modify, and distribute this software for any
// purpose is hereby granted without fee, provided that this copyright and
// permissions notice appear in all copies and derivatives.
//
// This software is provided "as is" without express or implied warranty.

#ifndef Reflex_PointerToMember
#define Reflex_PointerToMember

// Include files
#include "TypeBase.h"
#include "Reflex/Type.h"
#include "Reflex/Scope.h"

namespace Reflex {
namespace Internal {

   // forward declarations

   /** 
   * @class PointerToMember PointerToMember.h Reflex/PointerToMember.h
   * @author Stefan Roiser
   * @date   2004-01-28
   * @ingroup Ref
   */
   class PointerToMember : public TypeBase {

   public:

      /** default constructor */
      PointerToMember(const Type & pointerToMemberType,
         unsigned int modifiers,
         const Scope & pointerToMemberScope,
         const std::type_info & ti,
         const Catalog& catalog);


      /** destructor */
      virtual ~PointerToMember() {}


      /**
      * Name returns the name of the scope
      * @param  buf buffer to be used for calculating name
      * @param  mod qualifiers can be or'ed 
      *   kFinal     - resolve typedefs
      *   kScoped    - fully scoped name 
      *   kQualified - cv, reference qualification 
      * @return name of the type
      */
      const std::string& Name(std::string& buf, unsigned int mod = kScoped | kQualified) const;


      /**
      * PointerToMemberScope will return the scope of the pointer to member type
      * @return scope of the pointer to member type
      */
      Scope PointerToMemberScope() const;


      /** pointerToMemberType will return a pointer to the Type the pointer to
      * MemberAt points to
      * @return pointer to Type of pointer to MemberAt
      */
      Type ToType() const;


      /** static funtion that composes the At Name */
      static const std::string& BuildTypeName(std::string& buf, const Type & pointerToMemberType,
         const Scope & pointerToMemberScope,
         unsigned int mod = kScoped | kQualified);

      /** static funtion that composes the At Name */
      static const std::string& BuildTypeName(const Type & pointerToMemberType,
         const Scope & pointerToMemberScope,
         unsigned int mod = kScoped | kQualified) {
         std::string buf;
         return BuildTypeName(buf, pointerToMemberType, pointerToMemberScope, mod);
      }

   private:

      /**
      * The type of the pointer to member type
      * @link aggregation
      * @label pointer to member type
      * @supplierCardinality 1
      * @clientCardinality 1
      */
      Type fPointerToMemberType;


      /** 
      * The scope of the pointer to member type
      * @link aggregation
      * @label pointer to member scope
      * @supplierCardinality 1
      * @clientCardinality 1
      */
      Scope fPointerToMemberScope;


   }; // class PointerToMember
} // namespace Internal
} // namespace Reflex


//-------------------------------------------------------------------------------
inline Reflex::Scope
Reflex::Internal::PointerToMember::PointerToMemberScope() const {
//-------------------------------------------------------------------------------
   return fPointerToMemberScope;
}


//-------------------------------------------------------------------------------
inline Reflex::Type
Reflex::Internal::PointerToMember::ToType() const {
//-------------------------------------------------------------------------------
   return fPointerToMemberType;
}


#endif // Reflex_PointerToMember

