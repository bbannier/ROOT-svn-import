// @(#)root/reflex:$Id$
// Author: Philippe Bourgau 2009

// Copyright CERN, CH-1211 Geneva 23, 2004-2006, All rights reserved.
//
// Permission to use, copy, modify, and distribute this software for any
// purpose is hereby granted without fee, provided that this copyright and
// permissions notice appear in all copies and derivatives.
//
// This software is provided "as is" without express or implied warranty.

#ifndef Reflex_SharedLibraryUtils
#define Reflex_SharedLibraryUtils

#include "Reflex/Kernel.h"
#include <string>


namespace Reflex {


   /**
    * Path of the loaded shared library defining a function.
    * @param functionPointer a pointer to a function
    * @return a string with the path of the loaded sharedLibrary
    */
   RFLX_API std::string SharedLibraryDefining(const void* functionPointer);


   /**
    * Path of the loaded executable.
    * @return a string with the path of the loaded executable
    */
   RFLX_API std::string ExecutablePath();


   /**
    * Name part of a path
    * @param path a path to a file
    * @return a string with the base name of the file
    */
   RFLX_API std::string NamePartOfPath(const std::string& path);


   /**
    * Directory part of a path
    * @param path a path to a file
    * @return a string with the directory of the file, finished by a slash
    */
   RFLX_API std::string DirectoryPartOfPath(const std::string& path);


} // namespace Reflex



#endif // Reflex_SharedLibraryUtils
