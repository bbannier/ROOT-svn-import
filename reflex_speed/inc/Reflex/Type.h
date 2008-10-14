// @(#)root/reflex:$Id$
// Author: Stefan Roiser 2004, Axel Naumann 2008

// Copyright CERN, CH-1211 Geneva 23, 2004-2008, All rights reserved.
//
// Permission to use, copy, modify, and distribute this software for any
// purpose is hereby granted without fee, provided that this copyright and
// permissions notice appear in all copies and derivatives.
//
// This software is provided "as is" without express or implied warranty.

#ifndef Reflex_Type
#define Reflex_Type

// Include files
#include "Reflex/Kernel.h"
#include "Reflex/EntityProperty.h"
#include "Reflex/Container.h"
#include <vector>
#include <string>
#include <typeinfo>
#include <utility>

namespace Reflex {

   // forward declarations
   class Base;
   class Object;
   class PropertyList;
   class Scope;
   class TemplateArgument;
   class DictionaryGenerator;

   namespace Internal {
      class TypeBase;
      class TypeName;
   }

   /**
   * @class Type Type.h Reflex/Type.h
   * @ingroup Ref
   */
   class RFLX_API Type {

   public:

      enum ETYPE_MODIFICATION {
         kReplace = 0,
         kAppend  = 1,
         kMask    = 2
      };

      /** default constructor */
      Type(const Internal::TypeName * typName = 0,
         unsigned int modifiers = 0);


      /** copy constructor */
      Type(const Type & rh);


      /** 
      * copy constructor applying qualification 
      * @param rh the right hand type
      * @param modifiers to be applied
      * @param operation, the default is to replace the modifiers, you can also kAppend or kMask them.
      *        kMask removes the modifiers specified in the 2nd argument
      */
      Type(const Type & rh,
         unsigned int modifiers,
         ETYPE_MODIFICATION operation = kReplace);


      /** destructor */
      ~Type();


      /** 
      * assignment operator
      */
      Type & operator = (const Type & rh);


      /** 
      * equal operator 
      */
      bool operator == (const Type & rh) const;


      /** 
      * not equal operator
      */
      bool operator != (const Type & rh) const;


      /**
      * lesser than operator
      */
      bool operator < (const Type & rh) const; 


      /**
      * operator Scope will return the corresponding scope of this type if
      * applicable (i.e. if the Type is also a Scope e.g. Class, Union, Enum)
      */                                       
      operator Scope () const;


      /**
      * the bool operator returns true if the Type is resolved (implemented)
      * @return true if Type is implemented 
      */
      operator bool () const;

      /**
      * Allocate will reserve memory for the size of the object
      * @return pointer to allocated memory
      */
      void * Allocate() const;


      /** 
      * ArrayLength returns the size of the array (if the type represents one)
      * @return size of array
      */
      size_t ArrayLength() const;


      /**
      * Bases returns the collection of base class information
      * @return collection of base class information
      */
      const OrderedContainer<Reflex::Base> Bases() const;


      /**
      * CastObject an object from this class type to another one
      * @param  to is the class type to cast into
      * @param  obj the memory address of the object to be casted
      */
      Object CastObject(const Type & to, 
         const Object & obj) const;


      /**
      * Construct will call the constructor of a given type and allocate
      * the memory for it
      * @param  signature of the constructor
      * @param  values for parameters of the constructor
      * @param  mem place in memory for implicit construction
      * @return new object 
      */
      Object Construct(const Type & signature = Type(0,0),
         const std::vector < void * > & values = std::vector < void * > (),
         void * mem = 0) const;


      /**
      * Deallocate will deallocate the memory for a given object
      * @param  instance of the type in memory
      */
      void Deallocate(void * instance) const;


      /**
      * DeclaringScope will return the declaring socpe of this type
      * @return declaring scope of this type
      */
      Scope DeclaringScope() const;


      /**
      * Destruct will call the destructor of a type and remove its memory
      * allocation if desired
      * @param  instance of the type in memory
      * @param  dealloc for also deallacoting the memory
      */
      void Destruct(void * instance, 
         bool dealloc = true) const;


      /**
      * DynamicType is used to discover the dynamic type (useful in 
      * case of polymorphism)
      * @param  mem is the memory address of the object to checked
      * @return the actual class of the object
      */
      Type DynamicType(const Object & obj) const;


      /**
      * FinalType will return the type without typedefs 
      * @return type with all typedef info removed
      */
      Type FinalType() const;


      /**
      * FunctionParameters returns an ordered container of the type of the function parameters;
      * returns a reference to the dummy container if this type is not a function.
      * @return reflection information of nth function parameter
      */
      const OrderedContainer<Reflex::Type> FunctionParameters() const;


      /**
      * GenerateDict will produce the dictionary information of this type
      * @param generator a reference to the dictionary generator instance
      */
      void GenerateDict(DictionaryGenerator & generator) const;


      /* HasBase will check whether this class has a base class given
      * as argument
      * @param  cl the base-class to check for
      * @return the Base info if it is found, an empty base otherwise (can be tested for bool)
      */
      bool HasBase(const Type & cl) const;


      /**
      * Id returns a unique identifier of the type in the system
      * @return unique identifier
      */
      void * Id() const;


      /**
      * Check whether the entity property is set for the type. You can
      * combine checks, e.g. t.Is(kClass && kPublic)
      * @param descr the entity property to check for; see EntityProperty.
      * @return whether descr is set.
      */
      bool Is(const EntityProperty& descr) const;


      /** 
      * IsEquivalentTo returns true if the two types are equivalent
      * @param type to compare to
      * @modifiers_mask do not compare the listed modifiers
      * @return true if two types are equivalent
      */
      bool IsEquivalentTo(const Type & typ, 
         unsigned int modifiers_mask = 0) const;


      /** 
      * IsSignatureEquivalentTo returns true if the two types are equivalent,
      * ignoring the return type for functions
      * @param type to compare to
      * @modifiers_mask do not compare the listed modifiers
      * @return true if two types are equivalent
      */
      bool IsSignatureEquivalentTo(const Type & typ, 
         unsigned int modifiers_mask = 0) const;


      /** 
      * IsUnqualified returns true if the type represents an unqualified type
      * @return true if type represents an unqualified type
      */
      bool IsUnqualified() const;


      /**
      * Name returns the name of the type 
      * @param  mod qualifiers can be or'ed 
      *   kFinal     - resolve typedefs
      *   kScoped    - fully scoped name 
      *   kQualified - cv, reference qualification 
      * @return name of the type
      */
      std::string Name(unsigned int mod = kScoped | kQualified) const;


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
      * PointerToMemberScope will return the scope of the pointer to member type
      * @return scope of the pointer to member type
      */
      Scope PointerToMemberScope() const;


      /**
      * Properties will return a PropertyList attached to this item
      * @return PropertyList of this type
      */
      PropertyList Properties() const;


      /**
      * RawType will return the underlying type of a type removing all information
      * of pointers, arrays, typedefs
      * @return the raw type representation
      */
      Type RawType() const;


      /**
      * ReturnType will return the type of the return type
      * @return reflection information of the return type
      */
      Type ReturnType() const;


      /**
      * sizeof will return the size of the type
      * @return size of the type as int
      */
      size_t SizeOf() const;


      /**
      * TemplateArguments returns an ordered collection of the template arguments
      * @return reflection information of template arguments
      */
      const OrderedContainer<Reflex::TemplateArgument> TemplateArguments() const;


      /**
      * TemplateFamily returns the corresponding TypeTemplate if any
      * @return corresponding TypeTemplate
      */
      TypeTemplate TemplateFamily() const;


      /**
      * ToType will return an underlying type if possible (e.g. typedef, pointer..)
      * @return reflection information of underlying type
      */
      Type ToType() const;


      /**
      * Types returns the collection of currently defined types in
      * the system within the global catalog.
      * @return number of currently defined types
      */
      static const Container<Reflex::Type> Types();

      static Type ByName(const std::string& name) { return Types().ByName(name); }
      static Type ByTypeInfo(const std::type_info& ti) { return Types().ByTypeInfo(ti); }


      /**
      * TypeInfo will return the c++ type_info object of this type
      * @return type_info object of this type
      */
      const std::type_info & TypeInfo() const;


      /**
      * TypeType will return the enum information about this type
      * @return enum information of this type
      */
      ETYPE TypeType() const;


      /**
      * TypeTypeAsString will return the string representation of the kETEnum
      * representing the real type of the Type (e.g. "kETClass")
      * @return string representation of the ETYPE enum of the Type
      */
      std::string TypeTypeAsString() const;


      /**
      * Unload will unload the dictionary information of a type from the system
      */
      void Unload() const;


     /**
      * SetSize will set the size of the type. This function shall
      * be used with care. It will change the reflection information
      * of this type.
      */
      void SetSize(size_t s) const;


      /**
      * SetTypeInfo will set the type_info object of this type.
      * Attention: This will change the reflection information
      * of this type.
      */
      void SetTypeInfo(const std::type_info & ti) const;


      /** */
      const Internal::TypeBase * ToTypeBase() const;


   private:
      /** Internal function used to calculate the equality of types, used
          by IsEquivalentTo(), IsSignatureEquivalentTo().
       */
      bool DetermineEquivalence(const Type & typ, unsigned int modifiers_mask, bool alsoReturn) const;

   private:

      /** 
      * pointer to the TypeName 
      * @link aggregation
      * @supplierCardinality 1
      * @clientCardinality 1..
      **/
      const Internal::TypeName * fTypeName;


      /** modifiers */
      unsigned int fModifiers;

#ifdef REFLEX_CINT_MERGE
   public:
      // To prevent any un-authorized use as the old type
      bool operator!() const { return !operator bool(); }
      bool operator&&(bool right) const { return operator bool() && right; }
      bool operator&&(int right) const { return operator bool() && right; }
      bool operator&&(long right) const { return operator bool() && right; }
      bool operator&&(const Scope &right) const;
      bool operator&&(const Type &right) const;
      bool operator&&(const Member &right) const;
      bool operator||(bool right) const { return operator bool() || right; }
      bool operator||(int right) const { return operator bool() || right; }
      bool operator||(long right) const { return operator bool() || right; }
      bool operator||(const Scope &right) const;
      bool operator||(const Type &right) const;
      bool operator||(const Member &right) const;
   private:
      operator int () const;
#endif


   }; // class Type

} //namespace Reflex

//-------------------------------------------------------------------------------
inline Reflex::Type & Reflex::Type::operator = (const Type & rh) {
//-------------------------------------------------------------------------------
   fTypeName = rh.fTypeName;
   fModifiers = rh.fModifiers;
   return * this;
}


//-------------------------------------------------------------------------------
inline bool Reflex::Type::operator == (const Type & rh) const {
//-------------------------------------------------------------------------------
   return (fTypeName == rh.fTypeName && fModifiers == rh.fModifiers);
}


//-------------------------------------------------------------------------------
inline bool Reflex::Type::operator != (const Type & rh) const {
//-------------------------------------------------------------------------------
   return (fTypeName != rh.fTypeName || fModifiers != rh.fModifiers);
}


//-------------------------------------------------------------------------------
inline bool Reflex::Type::operator < (const Type & rh) const {
//-------------------------------------------------------------------------------
   return Id() < rh.Id();
}


//-------------------------------------------------------------------------------
inline Reflex::Type::Type(const Internal::TypeName * typName,
                                 unsigned int modifiers) 
//-------------------------------------------------------------------------------
   : fTypeName(typName),
     fModifiers(modifiers) {}


//-------------------------------------------------------------------------------
inline Reflex::Type::Type(const Type & rh)
//-------------------------------------------------------------------------------
   : fTypeName (rh.fTypeName),
     fModifiers (rh.fModifiers) {}


//-------------------------------------------------------------------------------
inline Reflex::Type::Type(const Type & rh, 
                                 unsigned int modifiers,
                                 ETYPE_MODIFICATION operation) 
//-------------------------------------------------------------------------------
   : fTypeName(rh.fTypeName),
     fModifiers(operation == kAppend ? rh.fModifiers | modifiers : 
                 (operation == kMask ? rh.fModifiers & (~modifiers): modifiers)) {}


//-------------------------------------------------------------------------------
inline Reflex::Type::~Type() {
//-------------------------------------------------------------------------------
}


//-------------------------------------------------------------------------------
inline void * Reflex::Type::Id() const {
//-------------------------------------------------------------------------------
   return (void*)fTypeName;
}


//-------------------------------------------------------------------------------
inline bool
Reflex::Type::IsEquivalentTo(const Type & typ, unsigned int modifiers_mask) const {
//-------------------------------------------------------------------------------
   return DetermineEquivalence(typ, modifiers_mask, true);
}


//-------------------------------------------------------------------------------
inline bool
Reflex::Type::IsSignatureEquivalentTo(const Type & typ, unsigned int modifiers_mask) const {
//-------------------------------------------------------------------------------
   return DetermineEquivalence(typ, modifiers_mask, false);
}


//-------------------------------------------------------------------------------
inline bool Reflex::Type::IsUnqualified() const {
//-------------------------------------------------------------------------------
   return 0 == fModifiers;
}


#ifdef REFLEX_CINT_MERGE
inline bool operator&&(bool b, const Reflex::Type & rh) {
   return b && rh.operator bool();
}
inline bool operator&&(int i, const Reflex::Type & rh) {
   return i && rh.operator bool();
}
inline bool operator||(bool b, const Reflex::Type & rh) {
   return b || rh.operator bool();
}
inline bool operator||(int i, const Reflex::Type & rh) {
   return i || rh.operator bool();
}
#endif
#endif // Reflex_Type
