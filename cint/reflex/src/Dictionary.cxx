// @(#)root/reflex:$Id$
// Author: Stefan Roiser 2004

// Copyright CERN, CH-1211 Geneva 23, 2004-2006, All rights reserved.
//
// Permission to use, copy, modify, and distribute this software for any
// purpose is hereby granted without fee, provided that this copyright and
// permissions notice appear in all copies and derivatives.
//
// This software is provided "as is" without express or implied warranty.

#ifndef REFLEX_BUILD
#define REFLEX_BUILD
#endif

#include "Reflex/Dictionary.h"
#include "Reflex/internal/Names.h"
#include "Reflex/Builder/DictionaryBuilder.h"

#include <set>
#include <algorithm>
#include <functional>
#include <stdexcept>


//-------------------------------------------------------------------------------
std::vector<Reflex::Names*> Reflex::Dictionary::NamesIn(const std::vector<Dictionary>& dictionaries) {
//-------------------------------------------------------------------------------
   std::vector<Names*> result(dictionaries.size());
   std::transform(dictionaries.begin(), dictionaries.end(), result.begin(), std::mem_fun_ref(&::Reflex::Dictionary::NamesGet));
   return result;
}


//-------------------------------------------------------------------------------
Reflex::Dictionary::Dictionary(const std::string& sharedLibraryName, const std::vector<Dictionary> & dictionaries )
//-------------------------------------------------------------------------------
: fNames ( new Names( sharedLibraryName, NamesIn(dictionaries))) {
}


//-------------------------------------------------------------------------------
Reflex::Dictionary Reflex::Dictionary::Main() {
//-------------------------------------------------------------------------------
   return Names::Main();
}

//-------------------------------------------------------------------------------
Reflex::Dictionary Reflex::Dictionary::Defining(const void* functionPointer) {
//-------------------------------------------------------------------------------
   return Names::Defining(functionPointer);
}


//-------------------------------------------------------------------------------
bool Reflex::Dictionary::ExistsForSharedLibrary(const std::string& sharedLibraryName) {
//-------------------------------------------------------------------------------
   return NULL != Names::BySharedLibrary(sharedLibraryName);
}


//-------------------------------------------------------------------------------
Reflex::Dictionary Reflex::Dictionary::BySharedLibrary(const std::string& sharedLibraryName) {
//-------------------------------------------------------------------------------

   Reflex::Names* names = Names::BySharedLibrary(sharedLibraryName);
   if (!names)
      throw std::domain_error("No reflex information available for shared library \""+sharedLibraryName+"\".");

   return *names;
}


//-------------------------------------------------------------------------------
Reflex::Dictionary::Dictionary() 
//-------------------------------------------------------------------------------
   : fNames(&Names::Main()) {}


//-------------------------------------------------------------------------------
const std::string& Reflex::Dictionary::SharedLibraryName () const {
//-------------------------------------------------------------------------------
   return fNames->SharedLibraryName();
}


