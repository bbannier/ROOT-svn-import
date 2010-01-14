// @(#)root/reflex:$Id$
// Author: Philippe Bourgau 2009

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

#include "Reflex/SharedLibraryUtils.h"
#include <assert.h>

#ifdef WIN32

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

   DWORD requested = 0;
   EnumProcessModules(GetCurrentProcess(), NULL, 0, &requested);
   HMODULE* modules = new HMODULE[requested];
   DWORD needed = 0;
   EnumProcessModules(GetCurrentProcess(), modules, requested, &needed);

   HMODULE hmodule = NULL;
   for(DWORD i = 0; i < needed; ++i)
   {
      MODULEINFO moduleInfos = {0};
      GetModuleInformation(GetCurrentProcess(), modules[i], &moduleInfos, sizeof(moduleInfos));

      if (((DWORD)moduleInfos.lpBaseOfDll < (DWORD)functionPointer) &&
          ((DWORD)functionPointer < (DWORD)moduleInfos.lpBaseOfDll + (DWORD)moduleInfos.SizeOfImage))
      {
         hmodule = modules[i];
         break;
      }
   }
   delete[] modules;

   return HModulePath(hmodule);
}


//-------------------------------------------------------------------------------
std::string Reflex::ExecutablePath() {
//-------------------------------------------------------------------------------

   HMODULE hmodule = GetModuleHandle(NULL);
   return HModulePath(hmodule);
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

#endif //WIN32

//-------------------------------------------------------------------------------
std::string Reflex::NamePartOfPath(const std::string& path) {
//-------------------------------------------------------------------------------

   size_t slash_index = path.rfind(PATH_SEPARATOR);
   return path.substr(slash_index+1);
}

//-------------------------------------------------------------------------------
std::string Reflex::DirectoryPartOfPath(const std::string& path) {
//-------------------------------------------------------------------------------

   size_t slash_index = path.rfind(PATH_SEPARATOR);
   return path.substr(0, slash_index+1);
}
