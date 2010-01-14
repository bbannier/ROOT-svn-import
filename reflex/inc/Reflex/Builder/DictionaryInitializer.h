// @(#)root/reflex:$Id$
// Author: Philippe Bourgau 2009

// Copyright CERN, CH-1211 Geneva 23, 2004-2006, All rights reserved.
//
// Permission to use, copy, modify, and distribute this software for any
// purpose is hereby granted without fee, provided that this copyright and
// permissions notice appear in all copies and derivatives.
//
// This software is provided "as is" without express or implied warranty.

#ifndef Reflex_DictionaryInitializer
#define Reflex_DictionaryInitializer

// Include this file to enforce the initialization of a dictionary for the
// current shared library.

#include <string>
#include <Reflex/Builder/DictionaryBuilder.h>
#include <Reflex/SharedLibraryUtils.h>

namespace {

#ifdef REFLEX_VERSIONING

   std::string CurrentSharedLibraryPath() {
      static const std::string result (Reflex::SharedLibraryDefining((const void*)&CurrentSharedLibraryPath));
      return result;
   }

   Reflex::Dictionary dictionary = ::Reflex::DictionaryBuilder(CurrentSharedLibraryPath());

#else

   Reflex::Dictionary dictionary = ::Reflex::DictionaryBuilderMain();

#endif

} // unamed namespace


#endif // Reflex_DictionaryInitializer
