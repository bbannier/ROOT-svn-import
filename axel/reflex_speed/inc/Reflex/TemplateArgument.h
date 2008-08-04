// @(#)root/reflex:$Id$
// Author: Axel Naumann, 2008

// Copyright CERN, CH-1211 Geneva 23, 2004-2008, All rights reserved.
//
// Permission to use, copy, modify, and distribute this software for any
// purpose is hereby granted without fee, provided that this copyright and
// permissions notice appear in all copies and derivatives.
//
// This software is provided "as is" without express or implied warranty.

#ifndef Reflex_TemplateArgument
#define Reflex_TemplateArgument

#include "Reflex/ValueObject.h"

namespace Reflex {
   ////////////////////////////////////////////////////////////////////////////////
   // TemplateArgument represents an argument of a template instance, or a
   // default value of a template. It can be either an int, an enum constant, or a type.
   ////////////////////////////////////////////////////////////////////////////////

   class TemplateArgument {
   public:
      enum EKindOf {
         kType, // template parameter of type "typename / class / template"
         kValue // non-type template parameter
      };

      TemplateArgument(Type type): fKindOf(kType), fType(type) {}
      TemplateArgument(Type type, const Object& value): fKindOf(kValue), fValue(value), fType(type) {}

      /**
      * Name returns the name of the type
      * @param  mod qualifiers can be or'ed 
      *   kFinal     - resolve typedefs
      *   kScoped    - fully scoped name 
      *   kQualified - cv, reference qualification 
      * @return name of template argument (type or value)
      */
      std::string Name(unsigned int mod = kScoped | kQualified) const;

      /**
      * Name returns the name of the type
      * @param buf preallocated buffer to work on when calculating the name
      * @param  mod qualifiers can be or'ed 
      *   kFinal     - resolve typedefs
      *   kScoped    - fully scoped name 
      *   kQualified - cv, reference qualification 
      * @return name of template argument (type or value)
      */
      virtual const std::string& Name(std::string& buf, unsigned int mod = kScoped | kQualified) const;

      EKindOf KindOf() const { return fKindOf; }
      const Object& AsValue() const { return fValue; }
      Type          AsType() const { return fType; }

   private:
      EKindOf     fKindOf; // type of template parameter
      Object      fValue;  // value for kValue; also used to store the type for kType.
      Type        fType;   // type of the value (for kValue) or type template argument (for kType)
   };
} // namespace Reflex;


//-------------------------------------------------------------------------------
inline std::string
Reflex::TemplateArgument::Name(unsigned int mod) const {
//-------------------------------------------------------------------------------
   std::string buf;
   return Name(buf, mod);
}

//-------------------------------------------------------------------------------
inline const std::string&
Reflex::TemplateArgument::Name(std::string& buf, unsigned int) const {
//-------------------------------------------------------------------------------
   if (fKindOf == kType)
      return AsType().Name(buf);
   return AsValue().ToString(buf);
}

#endif // Reflex_TemplateArgument
