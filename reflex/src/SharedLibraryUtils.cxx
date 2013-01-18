// @(#)root/reflex:$Id$
// Author: Philippe Bourgau 2009

// Copyright CERN, CH-1211 Geneva 23, 2004-2010, All rights reserved.
//
// Permission to use, copy, modify, and distribute this software for any
// purpose is hereby granted without fee, provided that this copyright and
// permissions notice appear in all copies and derivatives.
//
// This software is provided "as is" without express or implied warranty.

#ifndef REFLEX_BUILD
#define REFLEX_BUILD
#endif

#include "Reflex/SharedLibraryUtils.h"
#include <assert.h>
#include <stdio.h>
#ifndef _MSC_VER
#include <unistd.h>
#endif

namespace {

//-------------------------------------------------------------------------------
std::string Suffix(const std::string& path, char separator) {
//-------------------------------------------------------------------------------

   size_t split_index = path.rfind(separator);
   return path.substr(split_index+1);
}

}

#ifdef WIN32

#include <algorithm>
#include <Windows.h>
#include <Psapi.h>
#pragma comment(lib, "Psapi")

static const char PATH_SEPARATOR  = '\\';

namespace
{
   //-------------------------------------------------------------------------------
   std::string HModulePath(HMODULE hmodule) {
   //-------------------------------------------------------------------------------

      assert(hmodule);

      char result[MAX_PATH] = {0};
      GetModuleFileName(hmodule, result, MAX_PATH);

      return result;
   }
}

//-------------------------------------------------------------------------------
std::string Reflex::SharedLibraryDefining(const void* functionPointer) {
//-------------------------------------------------------------------------------

   HMODULE hmodule = NULL;
   GetModuleHandleEx(GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS | GET_MODULE_HANDLE_EX_FLAG_UNCHANGED_REFCOUNT,
                     (LPCTSTR)functionPointer,
                     &hmodule);
   assert(hmodule && "Could not find the dll containing pointer.");
   return HModulePath(hmodule);
}


//-------------------------------------------------------------------------------
std::string Reflex::ExecutablePath() {
//-------------------------------------------------------------------------------

   HMODULE hmodule = GetModuleHandle(NULL);
   return HModulePath(hmodule);
}

//-------------------------------------------------------------------------------
std::string Reflex::NamePartOfPath(const std::string& path) {
//-------------------------------------------------------------------------------

   std::string result = Suffix(path, PATH_SEPARATOR);
   std::transform(result.begin(), result.end(), result.begin(), tolower);
   return result;
}

#else // WIN32

#include <dlfcn.h>

static const char PATH_SEPARATOR  = '/';

//-------------------------------------------------------------------------------
std::string Reflex::SharedLibraryDefining(const void* functionPointer) {
//-------------------------------------------------------------------------------

   Dl_info moduleInfo = {0};
   dladdr((void*)functionPointer, &moduleInfo);
   return moduleInfo.dli_fname;
}

//-------------------------------------------------------------------------------
std::string Reflex::ExecutablePath() {
//-------------------------------------------------------------------------------

   char result[FILENAME_MAX];
   int len = readlink("/proc/self/exe", result, FILENAME_MAX - 1);
   assert(0 < len);
   result[len] = '\0';
   return result;
}

//-------------------------------------------------------------------------------
std::string Reflex::NamePartOfPath(const std::string& path) {
//-------------------------------------------------------------------------------

   return Suffix(path, PATH_SEPARATOR);
}

#endif //WIN32


//-------------------------------------------------------------------------------
std::string Reflex::DirectoryPartOfPath(const std::string& path) {
//-------------------------------------------------------------------------------

   size_t slash_index = path.rfind(PATH_SEPARATOR);
   return path.substr(0, slash_index+1);
}
