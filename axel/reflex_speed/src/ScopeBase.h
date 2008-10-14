// @(#)root/reflex:$Id$
// Author: Stefan Roiser 2004

// Copyright CERN, CH-1211 Geneva 23, 2004-2006, All rights reserved.
//
// Permission to use, copy, modify, and distribute this software for any
// purpose is hereby granted without fee, provided that this copyright and
// permissions notice appear in all copies and derivatives.
//
// This software is provided "as is" without express or implied warranty.

#ifndef Reflex_ScopeBase
#define Reflex_ScopeBase

// Include files
#include "Reflex/Kernel.h"
#include "Reflex/Scope.h"

#include "OwnedPropertyList.h"
#include "OwnedMember.h"
#include "OwnedMemberTemplate.h"
#include "ContainerImpl.h"
#include "OrderedContainerImpl.h"
#include <vector>

#ifdef _MSC_VER
#pragma warning(push)
#pragma warning(disable : 4251)
#endif

namespace Reflex {

   // forward declarations
   class Catalog;
   class Member;
   class TypeTemplate;
   class MemberTemplate;
   class Type;
   class DictionaryGenerator;

namespace Internal {

   // forward declarations
   class ScopeName;


   /**
   * @class ScopeBase ScopeBase.h Reflex/ScopeBase.h
   * @author Stefan Roiser
   * @date 24/11/2003
   * @ingroup Ref
   */
   class RFLX_API ScopeBase {

   public:

      typedef OrderedContainerImpl<std::string, Member> OrdMemberCont_t;
      typedef OrderedContainerImpl<std::string, OwnedMember> OrdOwnedMemberCont_t;
      typedef OrderedContainerImpl< std::string, Scope, kUnique> OrdScopeUniqueCont_t;
      typedef OrderedContainerImpl< std::string, Type > OrdTypeCont_t;
      typedef OrderedContainerImpl< std::string, Type, kUnique > OrdTypeUniqueCont_t;
      typedef OrderedContainerImpl< std::string, TypeTemplate > OrdTypeTemplateCont_t;
      typedef OrderedContainerImpl< std::string, OwnedMemberTemplate > OwnedMemberTemplateCont_t;
      typedef ContainerImpl< std::string, Scope > ScopeCont_t;

      /** constructor within a At*/
      ScopeBase(const char * scope,
                ETYPE scopeType,
                const Catalog& catalog);


      /** destructor */
      virtual ~ScopeBase();


      /** 
      * operator Scope will return the corresponding Scope object
      * @return Scope corresponding to this ScopeBase
      */
      operator Scope () const;


      /** 
      * the operator Type will return a corresponding Type object to the At if
      * applicable (i.e. if the Scope is also a Type e.g. Class, Union, Enum)
      */
      operator Type () const;


      /**
      * DataMembers returns the data members of the type
      * @return nth data member 
      */
      const OrdMemberCont_t& DataMembers() const;


      /**
      * DeclaringScope will return a pointer to the At of this one
      * @return pointer to declaring At
      */
      virtual Scope DeclaringScope() const;


      /**
      * FunctionMembers returns the collection of function members of the scope
      * @return reflection information of function members
      */
      const OrdMemberCont_t& FunctionMembers() const;


      /**
      * FunctionMemberByName will return the MemberAt with the Name, 
      * optionally the signature of the function may be given
      * @param  Name of function MemberAt
      * @param  signature of the MemberAt function
      * @modifiers_mask When matching, do not compare the listed modifiers
      * @return function MemberAt
      */
      Member FunctionMemberByName(const std::string & name,
         const Type & signature,
         unsigned int modifiers_mask = 0) const;


      /**
      * GenerateDict will produce the dictionary information of this type
      * @param generator a reference to the dictionary generator instance
      */
      virtual void GenerateDict(DictionaryGenerator &generator) const;


      /**
      * GlobalScope will return the global scope representation
      * @return global scope
      */
      static Scope GlobalScope();


      /**
      * Retrieve the Catalog containing the scope.
      */
      const Catalog& InCatalog() const;


      /**
      * Check whether the entity property is set for the scope. You can
      * combine checks, e.g. Is(kClass && kPublic)
      * @param descr the entity property to check for; see EntityProperty.
      * @param mod unused; for compatibility with Type::Is().
      * @return whether descr is set.
      */
      virtual bool Is(const EntityProperty& descr, unsigned int = 0) const;


      /* IsComplete will return true if all classes and base classes of this 
      * class are resolved and fully known in the system
      */
      virtual bool IsComplete() const;


      /**
      * IsTopScope will return true if the current At is the top
      * (Empty) namespace
      * @return true if current sope is top namespace
      */
      bool IsTopScope() const;


      /**
      * LookupMember will lookup a member in the current scope
      * @param nam the string representation of the member to lookup
      * @param current the current scope
      * @return if a matching member is found return it, otherwise return empty member
      */
      Member LookupMember(const std::string & nam,
         const Scope & current) const;


      /**
      * LookupType will lookup a type in the current scope
      * @param nam the string representation of the type to lookup
      * @param current the current scope
      * @return if a matching type is found return it, otherwise return empty type
      */
      Type LookupType(const std::string & nam,
         const Scope & current) const;


      /**
      * LookupType will lookup a scope in the current scope
      * @param nam the string representation of the scope to lookup
      * @param current the current scope
      * @return if a matching scope is found return it, otherwise return empty scope
      */
      Scope LookupScope(const std::string & nam,
         const Scope & current) const;


      /**
      * MemberByName will return the first MemberAt with a given Name
      * @param Name  MemberAt Name
      * @return pointer to MemberAt
      */
      Member MemberByName(const std::string & name,
         const Type & signature) const;


      /**
      * Members eturns the collection of members of the scope
      * @return reflection information members
      */
      const OrdOwnedMemberCont_t& Members() const;


      /** 
      * MemberTemplates returns the collection of function member templates of this scope
      * @return collection of member templates
      */
      const OwnedMemberTemplateCont_t& MemberTemplates() const;


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


      /** 
      * At will return the At Object of this ScopeBase
      * @return corresponding Scope
      */
      Scope ThisScope() const;


      /**
      * ScopeType will return which kind of At is represented
      * @return At of At
      */
      ETYPE ScopeType() const;


      /**
      * ScopeTypeAsString will return the string representation of the enum
      * representing the current Scope (e.g. "kETClass")
      * @return string representation of enum for Scope
      */
      std::string ScopeTypeAsString() const;



      /* SimpleName returns the name of the type as a reference. It provides a 
      * simplified but faster generation of a type name. Attention currently it
      * is not guaranteed that Name() and SimpleName() return the same character 
      * layout of a name (ie. spacing, commas, etc. )
      * @param pos will indicate where in the returned reference the requested name starts
      * @param mod The only 'mod' support is SCOPED
      * @return name of type
      */
      const std::string & SimpleName( size_t & pos, 
         unsigned int mod = 0 ) const;


      /**
      * SubScopes returns the collection of scopes declared in this scope.
      * @return container of reflection information of sub scopes
      */
      const OrdScopeUniqueCont_t& SubScopes() const;


      /**
      * SubScopeLevel will return the number of declaring scopes
      * this scope lives in.
      * @return number of declaring scopes above this scope.
      */ 
      size_t SubScopeLevel() const;


      /**
      * SubTypes returns the collection of type declared in this scope.
      * @return reflection information of sub types
      */
      const OrdTypeUniqueCont_t& SubTypes() const;


      /** 
      * SubTypeTemplates returns the collection of templated types declared within this scope
      * @return type templates
      */
      const OrdTypeTemplateCont_t& SubTypeTemplates() const;


      /**
      * UsingDirectives returns the using directives declared in this scope.
      * @return using directives
      */
      const ScopeCont_t& UsingDirectives() const;


      /**
      * UsingTypeDeclarations returns the using declarations of types of this scope.
      * @return using declarations of types
      */
      const OrdTypeCont_t& UsingTypeDeclarations() const;


      /**
      * UsingMemberDeclarations returns the using declarations of members of this scope.
      * @return using declarations of members
      */
      const OrdMemberCont_t& UsingMemberDeclarations() const;


   protected:

      /** protected constructor for initialisation of the global namespace */
      ScopeBase(const Catalog& catalog);

   public:

      /**
      * AddMember adds the information about a member
      * virtual so Class can keep track of constructors, destructors.
      * @param dm member to add
      */
      virtual void AddMember(const Member & dm) const;
      void AddMember(const char * name,
         const Type & type,
         size_t offset,
         unsigned int modifiers = 0) const;


      /**
      * AddMember will add the information about a function member.
      * virtual so Class can keep track of constructors, destructors.
      */
      virtual void AddMember(const char * name,
         const Type & type,
         StubFunction stubFP,
         void * stubCtx = 0,
         const char * params = 0,
         unsigned int modifiers = 0) const;


      void AddMember(const MemberTemplate & mt) const;


      /**
      * AddSubScope will add a sub-At to this one
      * @param sc pointer to Scope
      */
      void AddSubScope(const Scope & sc) const;
      void AddSubScope(const char * scope, ETYPE scopeType) const;


      /**
      * AddSubType will add a sub-At to this At
      * @param sc pointer to Type
      */
      void AddSubType(const Type & ty) const;
      void AddSubType(const char * type,
         size_t size,
         ETYPE typeType,
         const std::type_info & ti,
         unsigned int modifiers = 0) const;


      void AddSubTypeTemplate(const TypeTemplate & tt) const;


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
      * RemoveDataMember will remove the information about a data MemberAt
      * virtual so Class can keep track of constructors, destructors.
      * @param dm pointer to data MemberAt
      */
      virtual void RemoveMember(const Member & dm) const;


      /**
      * RemoveFunctionMember will remove the information about a function MemberAt
      * @param fm pointer to function MemberAt
      */
      void RemoveMember(const MemberTemplate & mt) const;


      /**
      * RemoveSubScope will remove a sub-At to this one
      * @param sc pointer to Scope
      */
      void RemoveSubScope(const Scope & sc) const;


      /**
      * RemoveSubType will remove a sub-At to this At
      * @param sc pointer to Type
      */
      void RemoveSubType(const Type & ty) const;


      void RemoveSubTypeTemplate(const TypeTemplate & tt) const;


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


      virtual void HideName() const;

      ScopeName* ToScopeName() const { return fScopeName; }

   private:

      /* no copying, intentionally not implemented */
      ScopeBase(const ScopeBase &);

      /* no assignment, intentionally not implemented */
      ScopeBase & operator = (const ScopeBase &);

   protected:

      /**
      * pointers to members
      * @label scope members
      * @link aggregationByValue
      * @supplierCardinality 0..*
      * @clientCardinality 1
      */
      mutable
         OrdOwnedMemberCont_t fMembers;

      /**
      * container with pointers to all data members in this scope
      * @label scope datamembers
      * @link aggregation
      * @clientCardinality 1
      * @supplierCardinality 0..*
      */
      mutable
         OrdMemberCont_t fDataMembers;

      /**
      * container with pointers to all function members in this scope
      * @label scope functionmembers
      * @link aggregation
      * @supplierCardinality 0..*
      * @clientCardinality 1
      */
      mutable
         OrdMemberCont_t fFunctionMembers;


   private:

      /**
      * pointer to the Scope Name
      * @label scope name
      * @link aggregation
      * @clientCardinality 1
      * @supplierCardinality 1
      */
      ScopeName * fScopeName;


      /**
      * Type of the scope
      * @link aggregation
      * @label scope type
      * @clientCardinality 1
      * @supplierCardinality 1
      */
      ETYPE fScopeType;


      /**
      * pointer to declaring Scope
      * @label declaring scope
      * @link aggregation
      * @clientCardinality 1
      * @supplierCardinality 1
      */
      Scope fDeclaringScope;


      /**
      * pointers to sub-scopes
      * @label sub scopes
      * @link aggregation
      * @supplierCardinality 0..*
      * @clientCardinality 1
      */
      mutable
         OrdScopeUniqueCont_t fSubScopes;


      /**
      * pointer to types
      * @label sub types
      * @link aggregation
      * @supplierCardinality 0..*
      * @clientCardinality 1
      */
      mutable
         OrdTypeUniqueCont_t fSubTypes;


      /**
      * container for type templates defined in this scope
      * @label type templates
      * @link aggregation
      * @supplierCardinality 0..*
      * @clientCardinality 1
      */
      mutable
         OrdTypeTemplateCont_t fTypeTemplates;


      /**
      * container for member templates defined in this scope
      * @label member templates
      * @link aggregation
      * @supplierCardinality 0..*
      * @clientCardinality 1
      */
      mutable
         OwnedMemberTemplateCont_t fMemberTemplates;


      /** 
      * container for using directives of this scope
      * @label using directives
      * @linkScope aggregation
      * @supplierCardinality 0..*
      * @clientCardinality 1
      */
      mutable
         ScopeCont_t fUsingDirectives;


      /** 
      * container for type using declarations of this scope
      * @label using directives
      * @linkScope aggregation
      * @supplierCardinality 0..*
      * @clientCardinality 1
      */
      mutable
         OrdTypeCont_t fTypeUsingDeclarations;


      /** 
      * container for member using declarations of this scope
      * @label using directives
      * @linkScope aggregation
      * @supplierCardinality 0..*
      * @clientCardinality 1
      */
      mutable
         OrdMemberCont_t fMemberUsingDeclarations;


      /**
      * pointer to the property list
      * @label propertylist
      * @link aggregationByValue
      * @clientCardinality 1
      * @supplierCardinality 1
      */
      OwnedPropertyList fPropertyList;


      /** 
      * The position where the unscoped Name starts in the scopename
      */
      size_t fBasePosition;

   }; // class ScopeBase
} //namespace Internal
} //namespace Reflex


//-------------------------------------------------------------------------------
inline const Reflex::Internal::ScopeBase::OrdMemberCont_t&
Reflex::Internal::ScopeBase::DataMembers() const {
//-------------------------------------------------------------------------------
   return fDataMembers;
}


//-------------------------------------------------------------------------------
inline Reflex::Scope
Reflex::Internal::ScopeBase::DeclaringScope() const {
//-------------------------------------------------------------------------------
   return fDeclaringScope;
}


//-------------------------------------------------------------------------------
inline const Reflex::Internal::ScopeBase::OrdMemberCont_t&
Reflex::Internal::ScopeBase::FunctionMembers() const {
//-------------------------------------------------------------------------------
   return fFunctionMembers;
}


//-------------------------------------------------------------------------------
inline bool
Reflex::Internal::ScopeBase::IsComplete() const {
//-------------------------------------------------------------------------------
   return true;
}


//-------------------------------------------------------------------------------
inline bool
Reflex::Internal::ScopeBase::Is(const EntityProperty& prop, unsigned int) const {
//-------------------------------------------------------------------------------
   return prop.Eval(0, fScopeType);
}


//-------------------------------------------------------------------------------
inline const Reflex::Internal::ScopeBase::OrdOwnedMemberCont_t&
Reflex::Internal::ScopeBase::Members() const {
//-------------------------------------------------------------------------------
   return fMembers;
}


//-------------------------------------------------------------------------------
inline const Reflex::Internal::ScopeBase::OwnedMemberTemplateCont_t&
Reflex::Internal::ScopeBase::MemberTemplates() const {
//-------------------------------------------------------------------------------
   return fMemberTemplates;
}


//-------------------------------------------------------------------------------
inline Reflex::ETYPE
Reflex::Internal::ScopeBase::ScopeType() const {
//-------------------------------------------------------------------------------
   return fScopeType;
}


//-------------------------------------------------------------------------------
inline const Reflex::Internal::ScopeBase::OrdScopeUniqueCont_t&
Reflex::Internal::ScopeBase::SubScopes() const {
//-------------------------------------------------------------------------------
   return fSubScopes;
}


//-------------------------------------------------------------------------------
inline const Reflex::Internal::ScopeBase::OrdTypeUniqueCont_t&
Reflex::Internal::ScopeBase::SubTypes() const {
//-------------------------------------------------------------------------------
   return fSubTypes;
}


//-------------------------------------------------------------------------------
inline const Reflex::Internal::ScopeBase::OrdTypeTemplateCont_t&
Reflex::Internal::ScopeBase::SubTypeTemplates() const {
//-------------------------------------------------------------------------------
   return fTypeTemplates;
}


//-------------------------------------------------------------------------------
inline const Reflex::Internal::ScopeBase::ScopeCont_t&
Reflex::Internal::ScopeBase::UsingDirectives() const {
//-------------------------------------------------------------------------------
   return fUsingDirectives;
}


//-------------------------------------------------------------------------------
inline const Reflex::Internal::ScopeBase::OrdTypeCont_t&
Reflex::Internal::ScopeBase::UsingTypeDeclarations() const {
//-------------------------------------------------------------------------------
   return fTypeUsingDeclarations;
}


//-------------------------------------------------------------------------------
inline const Reflex::Internal::ScopeBase::OrdMemberCont_t&
Reflex::Internal::ScopeBase::UsingMemberDeclarations() const {
//-------------------------------------------------------------------------------
   return fMemberUsingDeclarations;
}


#ifdef _MSC_VER
#pragma warning(pop)
#endif

#endif // Reflex_ScopeBase
