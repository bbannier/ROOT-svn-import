// @(#)root/reflex:$Id$
// Author: Stefan Roiser 2004

// Copyright CERN, CH-1211 Geneva 23, 2004-2006, All rights reserved.
//
// Permission to use, copy, modify, and distribute this software for any
// purpose is hereby granted without fee, provided that this copyright and
// permissions notice appear in all copies and derivatives.
//
// This software is provided "as is" without express or implied warranty.

#ifndef Reflex_MemberBase
#define Reflex_MemberBase

// Include files
#include "Reflex/Kernel.h"
#include "Reflex/PropertyList.h"
#include "Reflex/Type.h"
#include "Reflex/Scope.h"
#include "OwnedPropertyList.h"

namespace Reflex {

   // forward declarations
   class Object;
   class EntityProperty;
   class DictionaryGenerator;

namespace Internal {

   /**
   * @class MemberBase MemberBase.h Reflex/internal/MemberBase.h
   * @author Stefan Roiser
   * @date 24/11/2003
   * @ingroup Ref
   */
   class RFLX_API MemberBase {

   public:

      /** default constructor */
      MemberBase(const char *   name,
         const Type &   type,
         ETYPE           memberType,
         unsigned int   modifiers);


      /** destructor */
      virtual ~MemberBase();


      /**
      * operator member will return the member object of this MemberBase
      */
      operator Member () const;


      /** 
      * DeclaringScope will return the scope which the member lives in
      * @return the declaring scope of the member
      */
      Scope DeclaringScope() const;


      /** 
      * DeclaringType will return the type which the member lives in
      * @return the declaring type of the member
      */
      Type DeclaringType() const;


      /**
      * GenerateDict will produce the dictionary information of this type
      * @param generator a reference to the dictionary generator instance
      */
      virtual void GenerateDict(DictionaryGenerator & generator) const;


      /** Get the member value */
      virtual Object Get(const Object & obj) const;


      /** Invoke the member function */
      /*virtual Object Invoke(const Object & obj, 
      const std::vector < Object > & paramList) const;*/
      virtual Object Invoke(const Object & obj, 
         const std::vector < void * > & paramList = 
         std::vector<void*>()) const;


      /** Invoke the function (for static functions) */
      //virtual Object Invoke(const std::vector < Object > & paramList) const;
      virtual Object Invoke(const std::vector < void * > & paramList = 
         std::vector<void*>()) const;


      /**
      * Check whether the entity property is set for the member. You can
      * combine checks, e.g. t.Is(gClass && gPUBLIC)
      * @param descr the entity property to check for; see EntityProperty.
      * @return whether descr is set.
      */
      bool Is(const EntityProperty& descr) const;


      /** return the type of the member (function or data member) */
      ETYPE MemberType() const;


      /** returns the string representation of the member species */
      std::string MemberTypeAsString() const;


      /** return the name of the member */
      virtual const std::string& Name(std::string& buf, unsigned int mod = kScoped) const;


      /** return the offset of the member */
      virtual size_t Offset() const;


      /** number of parameters */
      virtual size_t FunctionParameterSize(bool required = false) const;


      /** 
      * FunctionParameterDefaultss returns the collaction of default values 
      * (as strings) for function parameters.
      * @return collection of default values for parameters
      */
      const OrderedContainer<std::string>& FunctionParameterDefaults() const;


      /** 
      * FunctionParametertNames returns a collection of the parameter names
      * @return parameter names
      */
      const OrderedContainer<std::string>& FunctionParameterNames() const;


      /**
      * Properties will return a pointer to the property list attached
      * to this item
      * @return pointer to property list
      */
      PropertyList Properties() const;


      /** Set the member value */
      virtual void Set(const Object & instance,
         const void * value) const;


      /** Set the type of the member */
      void SetScope(const Scope & scope) const;


      /** return the context of the member */
      virtual void * Stubcontext() const;


      /** return the pointer to the stub function */
      virtual StubFunction Stubfunction() const;


      /**
      * TemplateArguments returns an ordered collection of the template arguments
      * @return reflection information of template arguments
      */
      const OrderedContainer<TemplateArgument>& TemplateArguments() const;


      /**
      * TemplateFamily returns the corresponding MemberTemplate if any
      * @return corresponding MemberTemplate
      */
      virtual MemberTemplate TemplateFamily() const;


      /** return pointer to member type */
      Type TypeOf() const;


      /** 
      * UpdateFunctionParameterNames updates the names of parameters
      * @param  parameters new list of ';' separated parameter names, must not specify default values
      */
      virtual void UpdateFunctionParameterNames(const char* parameters);

   protected:

      /** 
      * CalculateBaseObject will calculate the inheritance between an object
      * and the local type if necessary
      * @param obj the object from which the calculation should start
      * @return memory address of new local object relative to obj
      */
      void * CalculateBaseObject(const Object & obj) const;

   protected:

      /**
      * characteristics of the Member
      * @label Member
      * @supplierCardinality 1
      * @link aggregation
      * @clientCardinality 1
      */
      Type fType;


      /** all modifiers of the member */
      unsigned int fModifiers;

   private:

      /** name of member */
      std::string fName;


      /**
      * scope of the member
      * @label member scope
      * @link aggregation
      * @supplierCardinality 1
      * @clientCardinality 1
      */
      mutable
         Scope fScope;


      /** 
      * the kind of member (data/function-member)
      * @label member type
      * @link aggregation
      * @clientCardinality 1
      * @supplierCardinality 1
      */
      ETYPE fMemberType;


      /**
      * property list 
      * @label propertylist
      * @link aggregationByValue
      * @clientCardinality 1
      * @supplierCardinality 0..1
      */
      OwnedPropertyList fPropertyList;


      /**
      * pointer back to the member object
      * @label this member
      * @link aggregation
      * @supplierCardinality 1
      * @clientCardinality 1
      */
      Member * fThisMember;

   }; // class Member

} //namespace Internal
} //namespace Reflex

#include "Reflex/Object.h"
#include "Reflex/MemberTemplate.h"


//-------------------------------------------------------------------------------
inline Reflex::Object
Reflex::Internal::MemberBase::Get(const Object & /* obj */) const {
//-------------------------------------------------------------------------------
   return Object();
}


//-------------------------------------------------------------------------------
inline Reflex::Object 
Reflex::Internal::MemberBase::Invoke(const Object & /* obj */ ,
                                  const std::vector < void * > & /* paramList */) const {
//-------------------------------------------------------------------------------
   return Object();
}


//-------------------------------------------------------------------------------
inline Reflex::Object 
Reflex::Internal::MemberBase::Invoke(const std::vector < void * > & /* paramList */) const {
//-------------------------------------------------------------------------------
   return Object();
}


//-------------------------------------------------------------------------------
inline Reflex::ETYPE
Reflex::Internal::MemberBase::MemberType() const {
//-------------------------------------------------------------------------------
   return fMemberType;
}


//-------------------------------------------------------------------------------
inline const std::string&
Reflex::Internal::MemberBase::Name(std::string& buf, unsigned int mod) const {
//-------------------------------------------------------------------------------
   if (mod & kScoped) {
      if (!DeclaringScope().IsTopScope()) {
         buf += DeclaringScope().Name(mod);
         buf += "::";
      }
   }
   buf += fName;
   return buf;
}


//-------------------------------------------------------------------------------
inline size_t
Reflex::Internal::MemberBase::Offset() const {
//-------------------------------------------------------------------------------
   return 0;
}


//-------------------------------------------------------------------------------
inline void
Reflex::Internal::MemberBase::Set(const Object & /* instance */,
                                   const void * /* value */) const {}
//-------------------------------------------------------------------------------


//-------------------------------------------------------------------------------
inline void
Reflex::Internal::MemberBase::SetScope(const Scope & scope) const {
//-------------------------------------------------------------------------------
   fScope = scope;
}


//-------------------------------------------------------------------------------
inline void *
Reflex::Internal::MemberBase::Stubcontext() const {
//-------------------------------------------------------------------------------
   return 0;
}


//-------------------------------------------------------------------------------
inline Reflex::StubFunction
Reflex::Internal::MemberBase::Stubfunction() const {
//-------------------------------------------------------------------------------
   return 0;
}


//-------------------------------------------------------------------------------
inline Reflex::MemberTemplate
Reflex::Internal::MemberBase::TemplateFamily() const {
//-------------------------------------------------------------------------------
   return Dummy::MemberTemplate();
}


//-------------------------------------------------------------------------------
inline Reflex::Type
Reflex::Internal::MemberBase::TypeOf() const {
//-------------------------------------------------------------------------------
   return fType;
}


//-------------------------------------------------------------------------------
inline void
Reflex::Internal::MemberBase::UpdateFunctionParameterNames(const char* /*parameters*/) {}
//-------------------------------------------------------------------------------



#endif // Reflex_MemberBase



