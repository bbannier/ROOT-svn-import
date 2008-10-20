// @(#)root/reflex:$Id: ScopedType.h 25003 2008-07-31 16:11:41Z axel $
// Author: Axel Naumann 2008

// Copyright CERN, CH-1211 Geneva 23, 2004-2006, All rights reserved.
//
// Permission to use, copy, modify, and distribute this software for any
// purpose is hereby granted without fee, provided that this copyright and
// permissions notice appear in all copies and derivatives.
//
// This software is provided "as is" without express or implied warranty.

#ifndef Reflex_ScopedType
#define Reflex_ScopedType

// Include files
#include "TypeBase.h"
#include "ScopeBase.h"

namespace Reflex {

   // forward declarations
   class DictionaryGenerator;

namespace Internal {

   /**
   * @class ScopedType ScopedType.h Reflex/ScopedType.h
   * @author Axel Naumann
   * @date 2008-08-01
   * @ingroup Ref
   */
   class ScopedType : public TypeBase, public ScopeBase {

   public:

      /** default constructor */
      ScopedType(const char * name,
                 unsigned int modifiers,
                 size_t size,
                 ETYPE type,
                 const std::type_info & ti,
                 Catalog catalog);

      /** destructor */
      virtual ~ScopedType() {}


      /**
      * DeclaringScope will return a pointer to the At of this one
      * @return pointer to declaring At
      */
      virtual Scope DeclaringScope() const;


      virtual void HideName() const;


      virtual bool Is(const AttributesExpression& descr, unsigned int mod = 0) const;

      /**
      * Name returns the name of the scope
      * @param  buf buffer to be used for calculating name
      * @param  mod qualifiers can be or'ed 
      *   kFinal     - resolve typedefs
      *   kScoped    - fully scoped name 
      *   kQualified - cv, reference qualification 
      * @return name of the type
      */
      virtual const std::string& Name(std::string& buf, unsigned int mod = kScoped | kQualified) const;


      /**
      * Properties will return a pointer to the PropertyNth list attached
      * to this item
      * @return pointer to PropertyNth list
      */
      virtual PropertyList Properties() const;

   }; // class ScopedType
} //namespace Internal
} //namespace Reflex


//-------------------------------------------------------------------------------
inline
Reflex::Internal::ScopedType::ScopedType(const char * name,
                                         unsigned int modifiers,
                                         size_t size,
                                         ETYPE type,
                                         const std::type_info & ti,
                                         Catalog catalog)
//-------------------------------------------------------------------------------
// Construct the dictionary information for a type that is also a scope
   : TypeBase(name, modifiers, size, type, ti, catalog),
   ScopeBase(name, type, catalog) {}


//-------------------------------------------------------------------------------
inline void
Reflex::Internal::ScopedType::HideName() const {
//-------------------------------------------------------------------------------
   TypeBase::HideName();
   ScopeBase::HideName();
}


//-------------------------------------------------------------------------------
inline Reflex::Scope
Reflex::Internal::ScopedType::DeclaringScope() const
{
//-------------------------------------------------------------------------------
   return ScopeBase::DeclaringScope();
}


//-------------------------------------------------------------------------------
inline bool
Reflex::Internal::ScopedType::Is(const AttributesExpression& descr, unsigned int mod) const
{
//-------------------------------------------------------------------------------
   return TypeBase::Is(descr, mod);
}


//-------------------------------------------------------------------------------
inline const std::string&
Reflex::Internal::ScopedType::Name(std::string& buf, unsigned int mod) const
{
//-------------------------------------------------------------------------------
   return ScopeBase::Name(buf, mod);
}


//-------------------------------------------------------------------------------
inline Reflex::PropertyList
Reflex::Internal::ScopedType::Properties() const {
//-------------------------------------------------------------------------------
   return ScopeBase::Properties();
}

#endif // Reflex_ScopedType

