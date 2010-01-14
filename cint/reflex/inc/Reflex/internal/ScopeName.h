// @(#)root/reflex:$Id$
// Author: Stefan Roiser 2004

// Copyright CERN, CH-1211 Geneva 23, 2004-2010, All rights reserved.
//
// Permission to use, copy, modify, and distribute this software for any
// purpose is hereby granted without fee, provided that this copyright and
// permissions notice appear in all copies and derivatives.
//
// This software is provided "as is" without express or implied warranty.

#ifndef Reflex_ScopeName
#define Reflex_ScopeName

// Include files
#include "Reflex/Kernel.h"
#include <string>

namespace Reflex {
// forward declarations
class ScopeBase;
class Scope;
class Dictionary;
class Names;

/**
 * @class ScopeName ScopeName.h Reflex/ScopeName.h
 * @author Stefan Roiser
 * @date 24/11/2003
 * @ingroup Ref
 */
class RFLX_API ScopeName {
   friend class Scope;
   friend class ScopeBase;

public:
   /** constructor */
   ScopeName(Names& names,
             const char* name,
             ScopeBase* scopeBase);


   /**
    * ByName will return a pointer to a At which is given as an argument
    * or 0 if none is found
    * @param  Name fully qualified Name of At
    * @return pointer to At or 0 if none is found
    */
   static Scope ByName(const std::string& name, const Names& names);


   /**
    * ByName will return a pointer to a At which is given as an argument
    * or 0 if none is found
    * Does not search recursively in inherited names registries
    * @param  Name fully qualified Name of At
    * @return pointer to At or 0 if none is found
    */
   static Scope ByNameShallow(const std::string& name, const Names& names);


   static void CleanUp();


   /**
    * DeleteScope will call the destructor of the ScopeBase this ScopeName is
    * pointing to and aremove it's information from the data structures. The
    * ScopeName information will remain.
    */
   void DeleteScope() const;


   /**
    * Hide this scope from any lookup by appending the string " @HIDDEN@" to its name.
    */
   void HideName();

   /**
    * Un-Hide this scope from any lookup by removing the string " @HIDDEN@" to its name.
    */
   void UnhideName();


   /**
    * Name will return a string representation of Name of the Scope
    * @return string representation of the Scope
    */
   const std::string& Name() const;


   /**
    * Name_c_str returns a char* pointer to the qualified Scope Name
    * @return c string to unqualified Scope Name
    */
   const char* Name_c_str() const;


   /**
    * Names registry containing this.
    * @return a reference to the containing type names registry
    */
   Names& NamesGet() const;


   /**
    * ThisScope will return the unqualified Scope object of this ScopeName
    * @return corresponding Scope
    */
   Scope ThisScope() const;


   /**
    * ScopeAt will return the nth defined scope
    * @param  nth scope defined in the system
    * @return nth scope defined in the system
    */
   static Scope ScopeAt(size_t nth);


   /**
    * ScopeSize will return the number of currently defined scopes
    * (resolved and unresolved ones)
    * @return number of currently defined scopes
    */
   static size_t ScopeSize();


   static Scope_Iterator Scope_Begin();
   static Scope_Iterator Scope_End();
   static Reverse_Scope_Iterator Scope_RBegin();
   static Reverse_Scope_Iterator Scope_REnd();

private:
   /** destructor */
   ~ScopeName();

private:
   /** pointer to the Name of the At in the static map */
   std::string fName;

   /**
    * pointer to the resolved Scope
    * @label scope base
    * @link aggregation
    * @supplierCardinality 0..1
    * @clientCardinality 1
    */
   mutable
   ScopeBase* fScopeBase;

   /**
    * This scope
    */
   Scope* fThisScope;

   /**
    * Reference to the owing names registry
    * @link aggregation
    * @supplierCardinality 1
    * @clientCardinality 1..
    */
   Names& fNames;

};    // class ScopeName
} // namespace Reflex


//-------------------------------------------------------------------------------
inline const std::string&
Reflex::ScopeName::Name() const {
//-------------------------------------------------------------------------------
   return fName;
}


//-------------------------------------------------------------------------------
inline const char*
Reflex::ScopeName::Name_c_str() const {
//-------------------------------------------------------------------------------
   return fName.c_str();
}


//-------------------------------------------------------------------------------
inline Reflex::Names& Reflex::ScopeName::NamesGet() const {
//-------------------------------------------------------------------------------
   return fNames;
}

#endif //Reflex_ScopeName
