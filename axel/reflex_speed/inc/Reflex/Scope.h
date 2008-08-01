// @(#)root/reflex:$Id$
// Author: Stefan Roiser 2004

// Copyright CERN, CH-1211 Geneva 23, 2004-2006, All rights reserved.
//
// Permission to use, copy, modify, and distribute this software for any
// purpose is hereby granted without fee, provided that this copyright and
// permissions notice appear in all copies and derivatives.
//
// This software is provided "as is" without express or implied warranty.

#ifndef Reflex_Scope
#define Reflex_Scope

// Include files
#include "Reflex/Kernel.h"
#include "Reflex/EntityProperty.h"
#include <string>
#include <typeinfo>

namespace Reflex {

   // forward declarations
   class Base;
   class EntityProperty;
   class Member;
   class PropertyList;
   class Type;
   class TypeTemplate;
   class MemberTemplate;
   class DictionaryGenerator;

   namespace Internal {
      class ScopeBase;
      class ScopeName;
   }


   /**
   * @class Scope Scope.h Reflex/Scope.h
   * @author Stefan Roiser
   * @date 24/11/2003
   * @ingroup Ref
   */
   class RFLX_API Scope {

   public:

      /** constructor */
      Scope(const Internal::ScopeName * scopeName = 0);


      /** copy constructor */
      Scope(const Scope & rh);


      /** destructor */
      ~Scope();


      /**
      * inequal operator 
      */
      bool operator != (const Scope & rh) const;


      /**
      * the bool operator will return true if the Scope is resolved (implemented)
      * @return true if Scope is implemented
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
      * the operator Type will return a corresponding type object to the scope if
      * applicable (i.e. if the Scope is also a Type e.g. Class, Union, Enum)
      */
      operator Type () const;


      /**
      * Bases returns the collection of base class information
      * @return collection of base class information
      */
      const OrderedContainer<Base> Bases() const;


      /**
      * Scopes returns the collection of reflection information of all the scopes
      * known by the global catalog.
      * @return collection of reflection information of all known scopes
      */
      static const Container<Scope> Scopes();

      static Scope ByName(const std::string& name) { return Scopes().ByName(name); }


      /**
      * DataMembers returns the data members of the type
      * @return nth data member 
      */
      const OrderedContainer<Member> DataMembers() const;


      /**
      * DeclaringScope will return the declaring socpe of this scope
      * @return declaring scope of this type
      */
      Scope DeclaringScope() const;


      /**
      * FunctionMembers returns the collection of function members of the scope
      * @return reflection information of nth function member
      */
      const OrderedContainer<Member> FunctionMembers() const;


      /**
      * FunctionMemberByName will return the member with the name, 
      * optionally the signature of the function may be given as a type
      * @param  name of function member
      * @param  signature of the member function 
      * @modifiers_mask When matching, do not compare the listed modifiers
      * @return reflection information of the function member
      */
      // this overloading is unfortunate but I can't include Type.h here
      Member FunctionMemberByName(const std::string & name,
         const Type & signature,
         unsigned int modifers_mask = 0) const;


      /**
      * FunctionMemberByNameAndSignature will return the member with the name, 
      * optionally the signature of the function may be given as a type
      * @param  name of function member
      * @param  signature of the member function 
      * @modifiers_mask When matching, do not compare the listed modifiers
      * @return reflection information of the function member
      */
      // this overloading is unfortunate but I can't include Type.h here
      Member FunctionMemberByNameAndSignature(const std::string & name,
         const Type & signature,
         unsigned int modifers_mask = 0) const;


      /**
      * GenerateDict will produce the dictionary information of this type
      * @param generator a reference to the dictionary generator instance
      */
      void GenerateDict(DictionaryGenerator & generator) const;


      /**
      * GlobalScope will return the global scope representation\
      * @return global scope
      */
      static Scope GlobalScope();


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
      * Check whether the entity property is set for the scope. You can
      * combine checks, e.g. Is(gClass && gPublic)
      * @param descr the entity property to check for; see EntityProperty.
      * @return whether descr is set.
      */
      bool Is(const EntityProperty& descr) const;


      /* IsComplete will return true if all classes and base classes of this 
      * class are resolved and fully known in the system
      */
      bool IsComplete() const;


      /** 
      * IsTopScope returns true if this scope is the top scope
      * @return true if this scope is the top scope
      */
      bool IsTopScope() const;


      /**
      * LookupMember will lookup a member in the current scope
      * @param nam the string representation of the member to lookup
      * @return if a matching member is found return it, otherwise return empty member
      */
      Member LookupMember(const std::string & nam) const;


      /**
      * LookupType will lookup a type in the current scope
      * @param nam the string representation of the type to lookup
      * @return if a matching type is found return it, otherwise return empty type
      */
      Type LookupType(const std::string & nam) const;


      /**
      * LookupScope will lookup a scope in the current scope
      * @param nam the string representation of the scope to lookup
      * @return if a matching scope is found return it, otherwise return empty scope
      */
      Scope LookupScope(const std::string & nam) const;


      /**
      * Members eturns the collection of members of the scope
      * @return reflection information members
      */
      const OrderedContainer<Member> Members() const;


      /**
      * MemberByName will return the first member with a given Name
      * @param  member name
      * @param  signature of the (function) member 
      * @return reflection information of the member
      */
      // this overloading is unfortunate but I can't include Type.h here
      Member MemberByName(const std::string & name,
         const Type & signature) const;


      /** 
      * MemberTemplates returns the collection of function member templates of this scope
      * @return collection of member templates
      */
      const OrderedContainer<MemberTemplate> MemberTemplates() const;


      /**
      * Name returns the name of the scope 
      * @param  mod qualifiers can be or'ed 
      *   kFinal     - resolve typedefs
      *   kScoped    - fully scoped name 
      *   kQualified - cv, reference qualification 
      * @return name of the type
      */
      std::string Name(unsigned int mod = kScoped | kQualified) const;


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
      * Properties will return a PropertyList attached to this item
      * @return PropertyList of this type
      */
      PropertyList Properties() const;


      /**
      * ScopeType will return the enum information about this scope
      * @return enum information of this scope
      */
      ETYPE ScopeType() const;


      /**
      * ScopeTypeAsString will return the string representation of the kEnum
      * representing the real type of the scope (e.g. "kClass")
      * @return string representation of the ETYPE enum of the scope
      */
      std::string ScopeTypeAsString() const;


      /**
      * SubScopes returns the number of declaring scopes for this scope; 0 is global scope.
      * @return nesting level of scope.
      */
      size_t SubScopeLevel() const;

      /**
      * SubScopes returns the collection of scopes declared in this scope.
      * @return container of reflection information of sub scopes
      */
      const OrderedContainer<Scope> SubScopes() const;


      /**
      * SubTypes returns the collection of type declared in this scope.
      * @return reflection information of sub types
      */
      const OrderedContainer<Type> SubTypes() const;


      /** 
      * SubTypeTemplates returns the collection of templated types declared within this scope
      * @return type templates
      */
      const OrderedContainer<TypeTemplate> SubTypeTemplates() const;


      /**
      * Unload will unload the dictionary information of a scope
      */
      void Unload() const;


      /**
      * UsingDirectives returns the using directives declared in this scope.
      * @return using directives
      */
      const Container<Scope> UsingDirectives() const;


      /**
      * UsingTypeDeclarations returns the using declarations of types of this scope.
      * @return using declarations of types
      */
      const Container<Type> UsingTypeDeclarations() const;


      /**
      * UsingMemberDeclarations returns the using declarations of members of this scope.
      * @return using declarations of members
      */
      const Container<Member> UsingMemberDeclarations() const;


   public:

      /**
      * AddMember adds the information about a member
      * @param dm member to add
      */
      void AddMember(const Member & dm) const;


      /**
      * AddMember adds the information about a data member
      * @param nam the name of the member
      * @param typ the type of the member
      * @param offs the offset of the member relative to the beginning of the scope
      * @param modifiers of the member
      */
      void AddMember(const char * name,
         const Type & type,
         size_t offset,
         unsigned int modifiers = 0) const;


      /**
      * AddMember will add the information about a function member
      * @param nam the name of the function member
      * @param typ the type of the function member
      * @param stubFP a pointer to the stub function
      * @param stubCtx a pointer to the context of the function member
      * @param params a semi colon separated list of parameters 
      * @param modifiers of the function member
      */ 
      void AddMember(const char * name,
         const Type & type,
         StubFunction stubFP,
         void * stubCtx = 0,
         const char * params = 0,
         unsigned int modifiers = 0) const;


      /** 
      * AddMember will add a member template to this scope
      * @param mt member template to add
      */
      void AddMember(const MemberTemplate & mt) const ;


      /**
      * AddSubScope will add a sub scope to this one
      * @param sc sub scope to add
      */
      void AddSubScope(const Scope & sc) const;


      /**
      * AddSubScope will add a sub scope to this one
      * @param scop the name of the sub scope
      * @param scopeType enum value of the scope type
      */
      void AddSubScope(const char * scope,
         ETYPE scopeType) const;


      /**
      * AddSubType will add a sub type to this type
      * @param ty sub type to add
      */
      void AddSubType(const Type & ty) const;


      /**
      * AddSubType will add a sub type to this type
      * @param typ the name of the sub type
      * @param size the sizeof of the sub type
      * @param typeType the enum specifying the sub type
      * @param ti the type_info of the sub type
      * @param modifiers of the sub type
      */
      void AddSubType(const char * type,
         size_t size,
         ETYPE typeType,
         const std::type_info & typeInfo,
         unsigned int modifiers = 0) const;


      /** 
      * AddTypeTemplate will add a sub type template to this scope
      * @param tt type template to add
      */
      void AddSubTypeTemplate(const TypeTemplate & mt) const ;


      /**
      * AddUsingDirective will add a using namespace directive to this scope
      * @param ud using directive to add
      */
      void AddUsingDirective(const Scope & ud) const;


      /**
      * AddUsingDeclaration adds a using declaration of a type to this scope
      * @param ud using declaration to add
      */
      void AddUsingDeclaration(const Type & ud) const;


      /**
      * AddUsingDeclaration adds a using declaration of a member to this scope
      * @param ud using declaration to add
      */
      void AddUsingDeclaration(const Member & ud) const;


      /**
      * RemoveMember removes the information about a member
      * @param dm member to remove
      */
      void RemoveMember(const Member & dm) const;


      /** 
      * RemoveMemberTemplate will remove a member template from this scope
      * @param mt member template to remove
      */
      void RemoveMember(const MemberTemplate & mt) const;


      /**
      * RemoveSubScope will remove a sub scope from this type
      * @param sc sub scope to remove
      */
      void RemoveSubScope(const Scope & sc) const;


      /**
      * RemoveSubType will remove a sub type from this type
      * @param sc sub type to remove
      */
      void RemoveSubType(const Type & ty) const;     


      /**
      * RemoveSubTypeTemplate will remove a sub type template from this scope
      * @param tt sub type template to remove
      */
      void RemoveSubTypeTemplate(const TypeTemplate & tt) const;


      /** 
      * RemoveUsingDirective will remove a using namespace directive from this scope
      * @param ud using namespace directive to remove
      */
      void RemoveUsingDirective(const Scope & ud) const;


      /** 
      * RemoveUsingDeclaration removes a using declaration from this scope
      * @param ud using declaration to remove
      */
      void RemoveUsingDeclaration(const Type & ud) const;


      /** 
      * RemoveUsingDeclaration removes a using declaration from this scope
      * @param ud using declaration to remove
      */
      void RemoveUsingDeclaration(const Member & ud) const;


      /** */
      const Internal::ScopeBase * ToScopeBase() const;

   public:

      /**
      * @label __NIRVANA__
      * @link association 
      */
      static Scope & __NIRVANA__();

   private:

      /**
      * pointer to the resolved scope
      * @label scope name
      * @link aggregation
      * @supplierCardinality 1
      * @clientCardinality 1
      */
      const Internal::ScopeName * fScopeName;

   }; // class Scope

   bool operator <(const Scope & lh, const Scope & rh); 

   bool operator==(const Scope & lh, const Scope & rh);

} // namespace Reflex

#include "Reflex/PropertyList.h"


//-------------------------------------------------------------------------------
inline bool Reflex::Scope::operator != (const Scope & rh) const {
//-------------------------------------------------------------------------------
   return (fScopeName != rh.fScopeName);
}


//-------------------------------------------------------------------------------
inline bool Reflex::operator < (const Scope & lh, const Scope & rh) {
//-------------------------------------------------------------------------------
   return const_cast<Scope*>(&lh)->Id() < const_cast<Scope*>(&rh)->Id();
}


//-------------------------------------------------------------------------------
inline bool Reflex::operator == (const Scope & lh,const Scope & rh) {
//-------------------------------------------------------------------------------
   return const_cast<Scope*>(&lh)->Id() == const_cast<Scope*>(&rh)->Id();
}


//-------------------------------------------------------------------------------
inline Reflex::Scope::Scope(const Internal::ScopeName * scopeName) 
//-------------------------------------------------------------------------------
   : fScopeName(scopeName) {}


//-------------------------------------------------------------------------------
inline Reflex::Scope::Scope(const Scope & rh) 
//-------------------------------------------------------------------------------
   : fScopeName(rh.fScopeName) {}


//-------------------------------------------------------------------------------
inline Reflex::Scope::~Scope() {
//-------------------------------------------------------------------------------
}


//-------------------------------------------------------------------------------
inline void * Reflex::Scope::Id() const {
//-------------------------------------------------------------------------------
   return (void*)fScopeName;
}


#ifdef REFLEX_CINT_MERGE
inline bool operator&&(bool b, const Reflex::Scope & rh) {
   return b && rh.operator bool();
}
inline bool operator&&(int i, const Reflex::Scope & rh) {
   return i && rh.operator bool();
}
inline bool operator||(bool b, const Reflex::Scope & rh) {
   return b || rh.operator bool();
}
inline bool operator||(int i, const Reflex::Scope & rh) {
   return i || rh.operator bool();
}
#endif

#endif // Reflex_Scope
