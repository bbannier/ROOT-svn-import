// @(#)root/reflex:$Id$
// Author: Philippe Bourgau 2009

// Copyright CERN, CH-1211 Geneva 23, 2004-2010, All rights reserved.
//
// Permission to use, copy, modify, and distribute this software for any
// purpose is hereby granted without fee, provided that this copyright and
// permissions notice appear in all copies and derivatives.
//
// This software is provided "as is" without express or implied warranty.

#ifndef Reflex_DictionaryBuilder
#define Reflex_DictionaryBuilder

// Include files
#include "Reflex/Dictionary.h"
#include <string>
#include <vector>


/**
 * Dictionary building helper functions.
 *
 * By default, reflex maintains a single type dictionary, containing all the types available in the exe.
 *
 * Reflex can be configured to handle a type defined differently inside two shared libraries.
 * To enable this:
 *   - compile shared libraries containing reflected information with the REFLEX_VERSIONING define
 *   - provide an implementation for the functions in Reflex::Versioning (see Reflex/Versioning.h)
 * With this mode enabled, only types loaded when the exe was loaded are visible from the main reflex
 * dictionary. Other dictionaries should be explicitly asked for (the same way dynamicaly loaded libraries
 * are programmaticaly accessed).
 *
 * NOTE: it is not recommended to mix default and versioned reflex dlls ...
 */

namespace Reflex {


   /**
    * Implementation function filling up a result list with the shared library on with "sharedLibrary" dependens on.
    * @param sharedLibraryPath the given shared library path
    * @param result the output variable where the dependencies are stored
    */
   RFLX_API void CollectSharedLibraryDependencies(const std::string& sharedLibraryPath, std::vector<std::string>& result);


   /**
    * Helper function building a dictionary for the specified shared library. If dictionaries for the
    * dependencies are not available, they will be created on the fly.
    * @param sharedLibraryPath path of the shared library for which to build a new dictionary
    * @return a valid Dictionary instance
    */
   RFLX_API Dictionary DictionaryBuilder(const std::string& sharedLibraryPath);

   /**
    * Helper function building the main dictionary.
    * @return a valid Dictionary instance
    */
   RFLX_API Dictionary DictionaryBuilderMain();

} // namespace Reflex 


#endif 
