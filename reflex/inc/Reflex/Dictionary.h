// @(#)root/reflex:$Id$
// Author: Stefan Roiser 2004

// Copyright CERN, CH-1211 Geneva 23, 2004-2006, All rights reserved.
//
// Permission to use, copy, modify, and distribute this software for any
// purpose is hereby granted without fee, provided that this copyright and
// permissions notice appear in all copies and derivatives.
//
// This software is provided "as is" without express or implied warranty.

#ifndef Reflex_Dictionary
#define Reflex_Dictionary

// Include files
#include "Reflex/Kernel.h"
#include "Reflex/SharedLibraryUtils.h"
#include <string>
#include <vector>
#include <typeinfo>


namespace Reflex {

   class Names;
   class Type;


   /**
   * @class Type Dictionary.h Reflex/Dictionary.h
   * @author Philippe Bourgau
   * @date 27/10/2009
   * @ingroup Ref
   *
   * By default, reflex maintains a single type dictionary, containing all the types available in the exe.
   *
   * Reflex can be configured to handle a type defined differently inside two shared libraries.
   * To enable this:
   *   - compile shared libraries containing reflected information with the REFLEX_VERSIONING define
   * With this mode enabled, only types loaded when the exe was loaded are visible from the main reflex
   * dictionary. Other dictionaries should be explicitly asked for (the same way dynamicaly loaded libraries
   * are programmaticaly accessed).
   *
   * NOTE: it is a very bad idea to mix default and versioned reflex dlls ...
   */
   class RFLX_API Dictionary {

      friend class Type;
      friend class Names;

   public:

      /** default constructor */
      Dictionary();


      /** main constructor */
      Dictionary( Names& names);


      /** copy constructor */
      Dictionary( const Dictionary & rh );


      /**
      * Inheritance constructor
      */
      Dictionary(const std::string& sharedLibraryName, const std::vector<Dictionary>& dictionaries);


      /** destructor */
      ~Dictionary();


      /** 
      * assignment operator
      */
      Dictionary & operator = ( const Dictionary & rh );


      /** 
      * equal operator 
      */
      bool operator == ( const Dictionary & rh ) const;


      /** 
      * not equal operator
      */
      bool operator != ( const Dictionary & rh ) const;


      /**
      * lesser than operator
      */
      bool operator < ( const Dictionary & rh) const;


      /**
      * Name of the shared library containing the types reflected by this.
      * @return a const reference to the name of the shared library
      */
      const std::string& SharedLibraryName () const;


      /**
      * Main dictionary where types from the main exe are registered
      */
      static Dictionary Main();


      /**
       * Dictionary containing the definition for type T.
       * @result a dictionary defining T
       */
      template<typename T>
      static Dictionary Defining();


      /**
       * Dictionary containing the definition for type ti.
       * @param ti type info for which to look for a dictionary
       * @result a dictionary defining ti
       */
      static Dictionary Defining(const std::type_info& ti);


      /**
       * Dictionary containing the definition for a function.
       * @param functionPointer a function for which to look for a dictionary
       * @result a dictionary defining the function
       */
      static Dictionary Defining(const void* functionPointer);


      /**
      * Is there a dictionary for the specified shared library ?
      */
      static bool ExistsForSharedLibrary(const std::string& sharedLibraryName);


      /**
      * Returns the dictionary for a given shared library.
      * @param sharedLibraryName name of the shared library
      * @return an instance of the dictionary
      * @throw domain_error if the shared library does not exist
      */
      static Dictionary BySharedLibrary(const std::string& sharedLibraryName);


   private:


      /**
      * Getter to the wrapped typenames pointer.
      * @return a pointer to the wrapped typename instance.
      */
      Names* NamesGet() const;


      /**
      * Extracts the names registries from the dictionaries and builds a vector with them.
      * @param dictionaries the dictionaries from which to extract the typenames
      * @return a stack allocated vector of pointers to the typenames
      */
      static std::vector<Names*> NamesIn(const std::vector<Dictionary>& dictionaries);

   private:

      /** 
      * pointer to the Names registry
      * @link aggregation
      * @supplierCardinality 1
      * @clientCardinality 1..
      **/
      Names * fNames;


   }; // class Dictionary


   /**
   * Dictionary for the current executable.
   */
   static Dictionary DictionaryExe();


} //namespace Reflex

//-------------------------------------------------------------------------------
inline Reflex::Dictionary & Reflex::Dictionary::operator = ( const Dictionary & rh ) {
//-------------------------------------------------------------------------------
   fNames = rh.fNames;
   return * this;
}


//-------------------------------------------------------------------------------
inline bool Reflex::Dictionary::operator == ( const Dictionary & rh ) const {
//-------------------------------------------------------------------------------
   return fNames == rh.fNames;
}


//-------------------------------------------------------------------------------
inline bool Reflex::Dictionary::operator != ( const Dictionary & rh ) const {
//-------------------------------------------------------------------------------
   return fNames != rh.fNames;
}


//-------------------------------------------------------------------------------
inline bool Reflex::Dictionary::operator < ( const Dictionary & rh ) const {
//-------------------------------------------------------------------------------
   return fNames < rh.fNames;
}


//-------------------------------------------------------------------------------
inline Reflex::Dictionary::Dictionary( Names& names ) 
//-------------------------------------------------------------------------------
   : fNames( &names ) {}


//-------------------------------------------------------------------------------
inline Reflex::Dictionary::Dictionary( const Dictionary & rh )
//-------------------------------------------------------------------------------
   : fNames ( rh.fNames ) {}


//-------------------------------------------------------------------------------
inline Reflex::Dictionary::~Dictionary() {
//-------------------------------------------------------------------------------
}


//-------------------------------------------------------------------------------
inline Reflex::Names* Reflex::Dictionary::NamesGet() const {
//-------------------------------------------------------------------------------
   return fNames;
}

//-------------------------------------------------------------------------------
template<typename T> inline Reflex::Dictionary Reflex::Dictionary::Defining() {
//-------------------------------------------------------------------------------
   return Dictionary::Defining(typeid(T));
}


//-------------------------------------------------------------------------------
inline Reflex::Dictionary Reflex::Dictionary::Defining(const std::type_info& ti) {
//-------------------------------------------------------------------------------
   return Dictionary::Defining(&ti);
}


//-------------------------------------------------------------------------------
inline Reflex::Dictionary Reflex::DictionaryExe() {
//-------------------------------------------------------------------------------

   // NOTE: this function is inline so that it can be correctly resolved from
   // the calling shared libraries (defining REFLEX_VERSIONING or not)

#ifdef REFLEX_VERSIONING

   return Dictionary::BySharedLibrary(ExecutablePath());

#else

   return Dictionary::Main();

#endif
}


#endif // Reflex_Dictionary
