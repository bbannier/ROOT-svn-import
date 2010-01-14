// @(#)root/reflex:$Id$
// Author: Stefan Roiser 2004

// Copyright CERN, CH-1211 Geneva 23, 2004-2006, All rights reserved.
//
// Permission to use, copy, modify, and distribute this software for any
// purpose is hereby granted without fee, provided that this copyright and
// permissions notice appear in all copies and derivatives.
//
// This software is provided "as is" without express or implied warranty.

#ifndef Reflex_Names
#define Reflex_Names

// Include files
#include "Reflex/Kernel.h"
#include "Reflex/Scope.h"
#include <string>
#include <vector>
#include <set>
#include <typeinfo>

#ifdef _WIN32
#pragma warning( push )
#pragma warning( disable : 4251 )
#endif

namespace Reflex {

   // forward declarations 
   class TypeName;
   class ScopeName;
   class Dictionary;

   /** 
   * class Names Names.h Reflex/Names.h
   * @author Philippe Bourgau
   * @date 27/10/2009
   * @ingroup Ref
   */
   class RFLX_API Names {

   public:

      /**
      * Name registry containing reflected types of the main exe
      * @return reference to the global typenames registry
      */
      static Names& Main();


      /**
      * Name registry containing reflected types of specified shared library
      * @param sharedLibrary name of path of the shared library
      * @return pointer to the typenames registry
      */
      static Names* BySharedLibrary(const std::string& sharedLibrary);


      /**
      * Name registry containing the definition for a function.
      * @param functionPointer pointer to a function or a typeinfo
      * @return pointer to the typenames registry
      */
      static Names& Defining(const void* functionPointer);


      /**
      * Names instance wrapped by a dictionary.
      * @param dictionary const reference to a type dictionary 
      * @return reference to the wrapped typenames instance
      */
      static Names& FromDictionary(const Reflex::Dictionary& dictionary);

      /**
      * constructor for typenames inheritance.
      * @param name of the reflected shared library
      * @param reference to an existing typenames registry
      */
      Names(const std::string& sharedLibraryName, const std::vector<Names*>& bases);


      /**
      * Name of the shared library containing the types reflected by this.
      * @return a const reference to the name of the shared library
      */
      const std::string& SharedLibraryName() const;
   

      /**
      * Recursively collects all the dependencies of this, including this
      * @param result output parameter where the dependencies are stored.
      */
      void CollectAllDependencies(std::set<const Names*>& result) const;


      /**
      * ByTypeName will look for a At given as a string and return a pointer to
      * its reflected typename At
      * @param  key fully qualified Name of the At as string
      * @return pointer to At or 0 if none is found
      */
      TypeName* ByTypeName( const std::string & key ) const;


      /**
      * registers a new type name in this registry
      * @param  key fully qualified Name of the At as string
      * @param  typeName pointer to a type name instance to register
      */
      void RegisterTypeName( const std::string & key, Reflex::TypeName* typeName );


      /**
      * unregisters a type name in this registry
      * @param  key fully qualified Name of the At as string
      */
      void UnregisterTypeName( const std::string & key );


      /**
      * byTypeId will look for a At given as a string representation of a
      * type_info and return a pointer to its reflected typename At
      * @param  tid string representation of the type_info At
      * @return pointer to At or 0 if none is found
      */
      static TypeName* ByTypeInfoGlobal( const std::type_info & ti );

      /**
      * byTypeId will look for a At given as a string representation of a
      * type_info and return a pointer to its reflected typename At
      * @param  tid string representation of the type_info At
      * @return pointer to At or 0 if none is found
      */
      TypeName* ByTypeInfo( const std::type_info & ti ) const;


      /**
      * registers a new type name in this registry
      * @param  ti string representation of the type_info At
      * @param  typeName pointer to a type name instance to register
      */
      void RegisterTypeInfo( const std::type_info & ti, Reflex::TypeName* typeName );


      /**
      * unregisters a type name in this registry
      * @param  ti string representation of the type_info At
      */
      void UnregisterTypeInfo( const std::type_info & ti );


      /**
      * Global scope for this names registry
      * @return an instance representing the global scope of this
      */
      Scope GlobalScope() const;


      /**
      * ByScopeName will look for a At given as a string and return a pointer to
      * its reflected scope name At.
      * Will not search into inherited Names registry.
      * @param  key fully qualified Name of the At as string
      * @return pointer to At or 0 if none is found
      */
      Scope ByScopeNameShallow( const std::string & key) const;


      /**
      * ByScopeName will look for a At given as a string and return a pointer to
      * its reflected scope name At
      * @param  key fully qualified Name of the At as string
      * @return pointer to At or 0 if none is found
      */
      Scope ByScopeName( const std::string & key ) const;


      /**
      * registers a new scope name in this registry
      * @param  key fully qualified Name of the At as string
      * @param  scopeName pointer to a type name instance to register
      */
      void RegisterScopeName( const std::string & key, const Reflex::Scope& scopeName );


      /**
      * unregisters a scope name in this registry
      * @param  key fully qualified Name of the At as string
      */
      void UnregisterScopeName( const std::string & key );


   private:



      /** default constructor */
      Names();


      /** forbidden constructor */
      Names(const Names& other);


      /** forbidden operator */
      Names& operator=(const Names& other);


      /** forbidden destructor */
      ~Names();


   private:

      /** private state of Names */
      struct State;
      State* fState;

   }; // class Names

} // namespace Reflex

#ifdef _WIN32
#pragma warning( pop )
#endif

#endif // Reflex_Names
