// @(#)root/reflex:$Id$
// Author: Stefan Roiser 2004

// Copyright CERN, CH-1211 Geneva 23, 2004-2006, All rights reserved.
//
// Permission to use, copy, modify, and distribute this software for any
// purpose is hereby granted without fee, provided that this copyright and
// permissions notice appear in all copies and derivatives.
//
// This software is provided "as is" without express or implied warranty.

#ifndef Reflex_TypeBase
#define Reflex_TypeBase

// Include files
#include "Reflex/Kernel.h"
#include "Reflex/Scope.h"
#include "OwnedPropertyList.h"
#include <vector>
#include <typeinfo>

namespace Reflex {

   // forward declarations
   class Member;
   class Base;
   class Type;
   class Object;
   class MemberTemplate;
   class TypeTemplate;
   class TemplateArgument;
   class DictionaryGenerator;

namespace Internal {

   // forward declarations
   class TypeName;

   /**
   * @class TypeBase TypeBase.h Reflex/TypeBase.h
   * @author Stefan Roiser
   * @date 24/11/2003
   * @ingroup Ref
   */
   class RFLX_API TypeBase {

   public:

      /** default constructor */
      TypeBase( const char *    nam, 
         size_t                 size, 
         TYPE                   typeTyp,
         const std::type_info & ti,
         const Type &           finalType = Dummy::Type());


      /** destructor */
      virtual ~TypeBase();


      /**
      * operator Scope will return the corresponding scope of this type if
      * applicable (i.e. if the Type is also a Scope e.g. Class, Union, Enum)
      */                                       
      operator Scope () const;


      /**
      * operator Type will return the corresponding Type object
      * @return Type corresponding to this TypeBase
      */
      operator Type () const;


      /**
      * Allocate will reserve memory for the size of the object
      * @return pointer to allocated memory
      */
      void * Allocate() const;


      /**
      * CastObject an object from this class At to another one
      * @param  to is the class At to cast into
      * @param  obj the memory AddressGet of the object to be casted
      */
      virtual Object CastObject( const Type & to, 
         const Object & obj ) const;


      /**
      * Construct will call the constructor of a given At and Allocate the
      * memory for it
      * @param  signature of the constructor
      * @param  values for parameters of the constructor
      * @param  mem place in memory for implicit construction
      * @return pointer to new instance
      */
      virtual Object Construct( const Type & signature,
         const std::vector < void * > & values,
         void * mem) const;


      /**
      * Deallocate will Deallocate the memory for a given object
      * @param  instance of the At in memory
      */
      void Deallocate( void * instance ) const;


      /**
      * Destruct will call the destructor of a At and remove its memory
      * allocation if desired
      * @param  instance of the At in memory
      * @param  dealloc for also deallacoting the memory
      */
      virtual void Destruct( void * instance, 
         bool dealloc = true ) const;


      /**
      * DeclaringScope will return a pointer to the At of this one
      * @return pointer to declaring At
      */
      virtual Scope DeclaringScope() const;


      /**
      * DynamicType is used to discover whether an object represents the
      * current class At or not
      * @param  mem is the memory AddressGet of the object to checked
      * @return the actual class of the object
      */
      virtual Type DynamicType( const Object & obj ) const;



      /**
      * FinalType will return the type without typedefs 
      * @return type with all typedef info removed
      */
      Type FinalType() const;


      /**
      * GenerateDict will produce the dictionary information of this type
      * @param generator a reference to the dictionary generator instance
      */
      virtual void GenerateDict(DictionaryGenerator &generator) const;


      /**
      * Check whether the entity property is set for the type. You can
      * combine checks, e.g. t.Is(gCLASS && gPUBLIC)
      * @param descr the entity property to check for; see EntityProperty.
      * @param mod the modifier as stored by Type (reference,...)
      * @return whether descr is set.
      */
      bool Is(const EntityProperty& descr, const int mod) const;


      /** 
      * IsComplete will return true if all classes and BaseAt classes of this 
      * class are resolved and fully known in the system
      */
      virtual bool IsComplete() const;


      /** Array
      * size returns the size of the array
      * @return size of array
      */
      virtual size_t ArrayLength() const;


      /**
      * Name returns the name of the type
      * @param buf preallocated buffer to work on when calculating the name
      * @return name of type
      */
      virtual const std::string& Name(std::string& buf, unsigned int mod = SCOPED | QUALIFIED ) const;


      /**
      * SimpleName returns the name of the type as a reference. It provides a 
      * simplified but faster generation of a type name. Attention currently it
      * is not guaranteed that Name() and SimpleName() return the same character 
      * layout of a name (ie. spacing, commas, etc. )
      * @param pos will indicate where in the returned reference the requested name starts
      * @param mod The only 'mod' support is SCOPED
      * @return name of type
      */
      virtual const std::string & SimpleName( size_t & pos, 
         unsigned int mod = SCOPED ) const;


      /**
      * FunctionParameters returns an ordered container of the type of the function parameters;
      * returns a reference to the dummy container if this type is not a function.
      * @return reflection information of nth function parameter
      */
      const OrderedContainer<Type> FunctionParameters() const;


      /**
      * PointerToMemberScope will return the scope of the pointer to member type
      * @return scope of the pointer to member type
      */
      virtual Scope PointerToMemberScope() const;


      /**
      * Properties will return a pointer to the PropertyNth list attached
      * to this item
      * @return pointer to PropertyNth list
      */
      virtual PropertyList Properties() const;


      /**
      * RawType will return the underlying type of a type removing all information
      * of pointers, arrays, typedefs
      * @return the raw type representation
      */
      Type RawType() const;


      /**
      * ReturnType will return a pointer to the At of the return At.
      * @return pointer to Type of return At
      */
      virtual Type ReturnType() const;


      /**
      * sizeof will return the size of the At
      * @return size of the At as int
      */
      size_t SizeOf() const;


      /**
      * TemplateArguments returns an ordered collection of the template arguments
      * @return reflection information of template arguments
      */
      const OrderedContainer<TemplateArgument>& TemplateArguments() const;


      /**
      * TemplateFamily returns the corresponding TypeTemplate if any
      * @return corresponding TypeTemplate
      */
      virtual TypeTemplate TemplateFamily() const;


      /**
      * arrayType will return a pointer to the At of the array.
      * @return pointer to Type of MemberAt et. al.
      */
      virtual Type ToType() const;


      /** 
      * At returns the corresponding unqualified Type object
      * @return corresponding At object
      */
      Type ThisType() const;


      /**
      * TypeInfo will return the c++ type_info object of the At
      * @return type_info object of At
      */
      virtual const std::type_info & TypeInfo() const;


      /**
      * TypeType will return the real At
      * @return real At
      */
      TYPE TypeType() const;


      /**
      * TypeTypeAsString will return the string representation of the TYPE At
      * @return string representation of TYPE At
      */
      std::string TypeTypeAsString() const;


   public:

      /**
      * AddMember will add the information about a data MemberAt
      * @param dm pointer to data MemberAt
      */
      virtual void AddMember( const Member & dm ) const;
      virtual void AddMember( const char * nam,
         const Type & typ,
         size_t offs,
         unsigned int modifiers = 0 ) const;


      /**
      * AddMember will add the information about a function MemberAt
      * @param fm pointer to function MemberAt
      */
      virtual void AddMember( const char * nam,
         const Type & typ,
         StubFunction stubFP,
         void * stubCtx = 0,
         const char * params = 0,
         unsigned int modifiers = 0 ) const;


      /**
      * RemoveMember will remove the information about a data MemberAt
      * @param dm pointer to data MemberAt
      */
      virtual void RemoveMember( const Member & dm ) const;


      /**
      * SetSize will set the size of the type. This function shall
      * be used with care. It will change the reflection information
      * of this type.
      */
      void SetSize( size_t s ) const;


      /**
      * SetTypeInfo will set the type_info object of this type.
      * Attention: This will change the reflection information
      * of this type.
      */
      void SetTypeInfo( const std::type_info & ti ) const;


      virtual void HideName() const;

   protected:

      /**
      * Retrieve the flags (bit mask of ENTITY_DESCRIPTION) for this type.
      */
      int TypeDescriptionFlags() const { return 0; }


      /**
      * DetermineFinalType will return the t without typedefs 
      * @return type with all typedef info removed
      */
      Type DetermineFinalType(const Type& t) const;

      /**
      * Pointer to the TypeName 
      * @label At Name
      * @ling aggregation
      * @link aggregation
      * @supplierCardinality 1
      * @clientCardinality 1
      */
      TypeName * fTypeName;


      /** C++ type_info object */
      mutable
         const std::type_info * fTypeInfo;

   private:

      /**
      * The Scope of the Type
      * @label type scope
      * @link aggregation
      * @clientCardinality 1
      * @supplierCardinality 1
      */
      Scope fScope;


      /** size of the type in int */
      mutable
         size_t fSize;


      /**
      * TYPE (kind) of the Type
      * @link aggregation
      * @label type type
      * @clientCardinality 1
      * @supplierCardinality 1
      */
      TYPE fTypeType;


      /**
      * Property list attached to this type
      * @label propertylist
      * @link aggregationByValue
      * @clientCardinality 1
      * @supplierCardinality 1
      */
      OwnedPropertyList fPropertyList;


      /**
      * The position where the unscoped Name starts in the typename
      */
      size_t fBasePosition;


      /**
      * the final type excluding typedefs
      * @label final typedef type
      * @link aggregation
      * @supplierCardinality 0..1
      * @clientCardinality 1
      */
      mutable
         Type * fFinalType;


      /**
      * the raw type excluding pointers, typedefs and arrays
      * @label raw type
      * @link aggregation
      * @supplierCardinality 0..1
      * @clientCardinality 1
      */
      mutable
         Type * fRawType;

   }; // class TypeBase
} //namespace Internal
} //namespace Reflex

#include "Reflex/TypeTemplate.h"


//-------------------------------------------------------------------------------
inline
Reflex::Internal::TypeBase::operator Reflex::Type () const {
//-------------------------------------------------------------------------------
// Converison operator to Type.
   return ThisType();
}


//-------------------------------------------------------------------------------
inline size_t
Reflex::Internal::TypeBase::ArrayLength() const {
//-------------------------------------------------------------------------------
// Return the length of the array type.
   return 0;
}


//-------------------------------------------------------------------------------
inline Reflex::Scope
Reflex::Internal::TypeBase::DeclaringScope() const {
//-------------------------------------------------------------------------------
// Return the declaring scope of this type.
   return fScope;
}


//-------------------------------------------------------------------------------
inline void
Reflex::Internal::TypeBase::Destruct( void * instance, 
                                       bool dealloc ) const {
//-------------------------------------------------------------------------------
// Destruct this type.
   if ( dealloc ) Deallocate(instance);
}


//-------------------------------------------------------------------------------
inline void
Reflex::Internal::TypeBase::GenerateDict( DictionaryGenerator & /* generator */ ) const {
//-------------------------------------------------------------------------------
// Generate Dictionary information about itself.
}

//-------------------------------------------------------------------------------
inline bool
Reflex::Internal::TypeBase::Is(const EntityProperty& prop, const int mod) const {
//-------------------------------------------------------------------------------
   return prop.Eval(TypeDescriptionFlags() | mod, fTypeType);
}


//-------------------------------------------------------------------------------
inline bool Reflex::Internal::TypeBase::IsComplete() const {
//-------------------------------------------------------------------------------
   return true;
}


//-------------------------------------------------------------------------------
inline Reflex::Scope
Reflex::Internal::TypeBase::PointerToMemberScope() const {
//-------------------------------------------------------------------------------
// Return the scope of a pointer to member type.
   return Dummy::Scope();
}


//-------------------------------------------------------------------------------
inline Reflex::PropertyList
Reflex::Internal::TypeBase::Properties() const {
//-------------------------------------------------------------------------------
// Return the property list attached to this type.
   return fPropertyList;
}


//-------------------------------------------------------------------------------
inline Reflex::Type
Reflex::Internal::TypeBase::ReturnType() const {
//-------------------------------------------------------------------------------
// Return the function return type.
   return Dummy::Type();
}


//-------------------------------------------------------------------------------
inline size_t Reflex::Internal::TypeBase::SizeOf() const { 
//-------------------------------------------------------------------------------
   return fSize; 
}


//-------------------------------------------------------------------------------
inline Reflex::TypeTemplate Reflex::Internal::TypeBase::TemplateFamily() const {
//-------------------------------------------------------------------------------
   return Dummy::TypeTemplate();
}


//-------------------------------------------------------------------------------
inline const std::type_info & Reflex::Internal::TypeBase::TypeInfo() const {
//-------------------------------------------------------------------------------
   return *fTypeInfo;
}


//-------------------------------------------------------------------------------
inline Reflex::TYPE
Reflex::Internal::TypeBase::TypeType() const {
//-------------------------------------------------------------------------------
// Return the kind of type as an enum.
   return fTypeType;
}


//-------------------------------------------------------------------------------
inline std::string
Reflex::Internal::TypeBase::TypeTypeAsString() const {
//-------------------------------------------------------------------------------
// Return the kind of type as a string.
   const std::string& s = TYPEName(fTypeType);
   if (s.empty())
      return "Type " + Name() + "is not assigned to a TYPE";
   return s;
}


//-------------------------------------------------------------------------------
inline void Reflex::Internal::TypeBase::AddMember( const Member & /* dm */ ) const {
//-------------------------------------------------------------------------------
   throw RuntimeError("Function AddMember  not callable on this object");
}


//-------------------------------------------------------------------------------
inline void Reflex::Internal::TypeBase::AddMember( const char * /* nam */,
                                                   const Type & /* typ */,
                                                   size_t /* offs */,
                                                   unsigned int /* modifiers */ ) const {
//-------------------------------------------------------------------------------
   throw RuntimeError("Function AddMember  not callable on this object");
}


//-------------------------------------------------------------------------------
inline void Reflex::Internal::TypeBase::AddMember( const char * /* nam */,
                                                   const Type & /* typ */,
                                                   StubFunction /* stubFP */,
                                                   void * /* stubCtx */,
                                                   const char * /* params */,
                                                   unsigned int /* modifiers */ ) const {
//-------------------------------------------------------------------------------
   throw RuntimeError("Function AddMember not callable on this object");
}


//-------------------------------------------------------------------------------
inline void Reflex::Internal::TypeBase::RemoveMember( const Member & /* dm */ ) const {
//-------------------------------------------------------------------------------
   throw RuntimeError("Function RemoveMember not callable on this object");
}


//-------------------------------------------------------------------------------
inline void Reflex::Internal::TypeBase::SetSize( size_t s ) const {
//-------------------------------------------------------------------------------
   fSize = s;
}


//-------------------------------------------------------------------------------
inline void Reflex::Internal::TypeBase::SetTypeInfo( const std::type_info & ti ) const {
//-------------------------------------------------------------------------------
   fTypeInfo = &ti;
}

#endif // Reflex_TypeBase



