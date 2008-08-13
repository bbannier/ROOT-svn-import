// @(#)root/reflex:$Id$
// Author: Stefan Roiser 2004

// Copyright CERN, CH-1211 Geneva 23, 2004-2006, All rights reserved.
//
// Permission to use, copy, modify, and distribute this software for any
// purpose is hereby granted without fee, provided that this copyright and
// permissions notice appear in all copies and derivatives.
//
// This software is provided "as is" without express or implied warranty.

#ifndef Reflex_Member
#define Reflex_Member

// Include files
#include "Reflex/Kernel.h"
#include "Reflex/Type.h"


namespace Reflex {

   // forward declarations
   class Type;
   class Scope;
   class EntityProperty;
   class PropertyList;
   class Object;
   class MemberTemplate;
   class TemplateArgument;
   class DictionaryGenerator;

   namespace Internal {
      class MemberBase;
      class OwnedMember;
   }

   /**
   * @class Member Member.h Reflex/Member.h
   * @author Stefan Roiser
   * @date 24/11/2003
   * @ingroup Ref
   */
   class RFLX_API Member {

      friend class Internal::OwnedMember;

   public:

      /** default constructor */
      Member(const Internal::MemberBase * memberBase = 0);


      /** copy constructor */
      Member(const Member & rh);


      /** destructor */
      ~Member();


      /**
      * lesser than operator 
      */
      bool operator < (const Member & rh) const;


      /** 
      * equal operator 
      */
      bool operator == (const Member & rh) const;


      /**
      * not equal operator 
      */
      bool operator != (const Member & rh) const;


      /** 
      * assignment operator 
      */
      Member & operator = (const Member & rh);


      /** 
      * operator bool will return true if the member is valid
      * @return true if member is implemented
      */
      operator bool () const;

#ifdef REFLEX_CINT_MERGE
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
   public:
#endif

      /** 
      * DeclaringScope will return the scope which the member lives in
      * @return the declaring scope of the member
      */
      Scope DeclaringScope() const;


      /** 
      * DeclaringType will return the type which the member lives in
      * (i.e. the same as the Scope)
      * @return the declaring type of the member
      */
      Type DeclaringType() const;


      /**
      * GenerateDict will produce the dictionary information of this type
      * @param generator a reference to the dictionary generator instance
      */
      void GenerateDict(DictionaryGenerator &generator) const;


      /** 
      * Get a static data member value 
      * @return member value as object
      */
      Object Get() const;


      /** 
      * Get the data member value 
      * @return member value as object
      */
      Object Get(const Object & obj) const;


      /**
      * Id returns a unique identifier of the member in the system
      * @return unique identifier
      */
      void * Id() const;


      /** 
      * Invoke a member function
      * @param obj the object which owns the member function
      * @param paramList a vector of addresses to paramter values
      * @return the return value of the function as object
      */
      Object Invoke(const Object & obj, 
         const std::vector < void * > & paramList = 
         std::vector<void*>()) const;


      /** 
      * Invoke a static function 
      * @param paramList a vector of addresses to parameter values
      * @return the return value of the function as object
      */
      Object Invoke(const std::vector < void * > & paramList = 
         std::vector<void*>()) const;


      /**
      * Check whether the entity property is set for the member. You can
      * combine checks, e.g. t.Is(gClass && gPublic)
      * @param descr the entity property to check for; see EntityProperty.
      * @return whether descr is set.
      */
      bool Is(const EntityProperty& descr) const;


      /** 
      * MemberType return the type of the member as enum value (function or data member) 
      * @return member type as enum
      */
      ETYPE MemberType() const;


      /** 
      * MemberTypeAsString returns the string representation of the member species 
      * @return member type as string representation
      */
      std::string MemberTypeAsString() const;


      /** 
      * Name returns the Name of the member 
      * @param mod modifiers can be or'ed as argument
      * kScoped - fully scoped name
      * @return name of the member
      */
      const std::string& Name(std::string& buf, unsigned int mod = kScoped) const;


      /** 
      * Name returns the Name of the member 
      * @param mod modifiers can be or'ed as argument
      * kScoped - fully scoped name
      * kFinal  - resolve all typedefs
      * kQualified - cv and reference qualification
      * @return name of the member
      */
      std::string Name(unsigned int mod = kScoped | kQualified) const;


      /** 
      * Offset returns the offset of the data member relative to the start of the scope
      * @return offset of member as int
      */
      size_t Offset() const;


      /** 
      * FunctionParameterSize returns the number of parameters 
      * @param required if true only returns the number of required parameters
      * @return number of parameters
      */
      size_t FunctionParameterSize(bool required = false) const;


      /** 
      * FunctionParameterDefaultss returns the collaction of default values 
      * (as strings) for function parameters.
      * @return collection of default values for parameters
      */
      const OrderedContainer<std::string> FunctionParameterDefaults() const;


      /** 
      * FunctionParametertNames returns a collection of the parameter names
      * @return parameter names
      */
      const OrderedContainer<std::string> FunctionParameterNames() const;


      /**
      * Properties will return the properties attached to this item
      * @return properties of this member
      */
      PropertyList Properties() const;


      /*void Set(const Object & instance,
      const Object & value) const;*/
      /**
      * Set will set the value of a data member
      * @param instance of the object owning the data member
      * @param value the memory address of the value to set
      */
      void Set(const Object & instance,
         const void * value) const;


      /** 
      * SetScope will set the Scope of the member 
      * @param sc scope to set
      */
      void SetScope(const Scope & sc) const;


      /** 
      * Stubcontext returns a pointer to the context of the member 
      * @return pointer to member context
      */
      void * Stubcontext() const;


      /** 
      * Stubfunction returns the pointer to the stub function 
      * @return function pointer to stub function
      */
      StubFunction Stubfunction() const;


      /**
      * TemplateArguments returns an ordered collection of the template arguments
      * @return reflection information of template arguments
      */
      const OrderedContainer<Reflex::TemplateArgument> TemplateArguments() const;


      /**
      * TemplateFamily returns the corresponding MemberTemplate if any
      * @return corresponding MemberTemplate
      */
      MemberTemplate TemplateFamily() const;


      /** 
      * TypeOf returns the member type 
      * @return member type
      */
      Type TypeOf() const;

      /** 
      * UpdateFunctionParameterNames updates the names of parameters
      * @param  parameters new list of ';' separated parameter names, must not specify default values
      */
      void UpdateFunctionParameterNames(const char* parameters);

   private:

      void Delete();

      /** 
      * the pointer to the member implementation 
      * @link aggregation
      * @supplierCardinality 1
      * @clientCardinality 0..1
      * @label member base
      */
      Internal::MemberBase * fMemberBase;

   }; // class Member

} //namespace Reflex



//-------------------------------------------------------------------------------
inline
Reflex::Member::Member(const Internal::MemberBase * memberBase)
//-------------------------------------------------------------------------------
   : fMemberBase(const_cast<Internal::MemberBase*>(memberBase)) {
   // Construct a member, attaching it to MemberBase.
}


//-------------------------------------------------------------------------------
inline
Reflex::Member::Member(const Member & rh)
//-------------------------------------------------------------------------------
   : fMemberBase(rh.fMemberBase) {
   // Member copy constructor.
}


//-------------------------------------------------------------------------------
inline
Reflex::Member::~Member() {
//-------------------------------------------------------------------------------
// Member desructor.
}


//-------------------------------------------------------------------------------
inline bool
Reflex::Member::operator < (const Member & rh) const {
//-------------------------------------------------------------------------------
   if ((*this) && rh) 
      return (TypeOf() < rh.TypeOf() && Name() < rh.Name());
   return false;
}


//-------------------------------------------------------------------------------
inline bool
Reflex::Member::operator == (const Member & rh) const {
//-------------------------------------------------------------------------------
   if ((*this) && rh) 
      return (TypeOf() == rh.TypeOf() && Name() == rh.Name());
   // both invalid is equal, too
   return (!(*this)) && (!rh);
}


//-------------------------------------------------------------------------------
inline bool
Reflex::Member::operator != (const Member & rh) const {
//-------------------------------------------------------------------------------
   return ! (*this == rh);
}


//-------------------------------------------------------------------------------
inline Reflex::Member &
Reflex::Member::operator = (const Member & rh) {
//-------------------------------------------------------------------------------
   fMemberBase = rh.fMemberBase;
   return * this;
}


//-------------------------------------------------------------------------------
inline
Reflex::Member::operator bool () const {
//-------------------------------------------------------------------------------
   return 0 != fMemberBase;
}


//-------------------------------------------------------------------------------
inline void *
Reflex::Member::Id() const {
//-------------------------------------------------------------------------------
   return (void*)fMemberBase;
}


//-------------------------------------------------------------------------------
inline std::string
Reflex::Member::Name(unsigned int mod) const {
//-------------------------------------------------------------------------------
   std::string buf;
   return Name(buf, mod);
}


#ifdef REFLEX_CINT_MERGE
inline bool operator&&(bool b, const Reflex::Member & rh) {
   return b && rh.operator bool();
}
inline bool operator&&(int i, const Reflex::Member & rh) {
   return i && rh.operator bool();
}
inline bool operator||(bool b, const Reflex::Member & rh) {
   return b || rh.operator bool();
}
inline bool operator||(int i, const Reflex::Member & rh) {
   return i || rh.operator bool();
}
#endif


#endif // Reflex_Member



