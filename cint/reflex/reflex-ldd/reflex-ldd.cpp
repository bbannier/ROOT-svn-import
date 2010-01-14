//==================================
// PEDUMP - Matt Pietrek 1994-2001
// FILE: PEDUMP.CPP
//==================================

#include <windows.h>
#include <iostream>
#include <fstream>
#include <string>
#include <exception>
#include <assert.h>
#include "bindeps.h"

#define IMAGE_SIZEOF_ROM_OPTIONAL_HEADER 56


template<typename T>
class AutoHandle
{
public:
   typedef BOOL (__stdcall *CloseFunction)(T);

   AutoHandle(T handle, CloseFunction close)
      : fHandle(handle), fClose(close)
   {
      assert(close);
   }
   ~AutoHandle()
   {
      if (fHandle)
         fClose(fHandle);
   }

   operator T()
   {
      return fHandle;
   }

private:
   T fHandle;
   CloseFunction fClose;
};



void CollectDependentDlls(const std::string& filename, std::vector<std::string>& result)
{
   AutoHandle<HANDLE> hFile (CreateFile(filename.c_str(), GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, 0), &CloseHandle);
   if (INVALID_HANDLE_VALUE == hFile)
      throw std::exception("Couldn't open file with CreateFile()");

   AutoHandle<HANDLE> hFileMapping (CreateFileMapping(hFile, NULL, PAGE_READONLY, 0, 0, NULL), &CloseHandle);
   if (!hFileMapping)
      throw std::exception("Couldn't open file mapping with CreateFileMapping()");

   AutoHandle<LPCVOID> pMappedFileBase (MapViewOfFile(hFileMapping,FILE_MAP_READ,0,0,0), &UnmapViewOfFile);
   if (!pMappedFileBase)
      throw std::exception("Couldn't map view of file with MapViewOfFile()");

   PIMAGE_DOS_HEADER dosHeader = (PIMAGE_DOS_HEADER)(LPCVOID)pMappedFileBase;
   if (IMAGE_DOS_SIGNATURE != dosHeader->e_magic)
      throw std::exception("unrecognized file format");

   CollectDependentDlls( dosHeader, result);
}

int main(int argc, char *argv[])
{
   if ( argc != 3 )
   {
      const char HelpText[] = 
         "dlldeps - Win32/Win64 EXE/DLL file dumper - 2001 Matt Pietrek\n\n"
         "Syntax: dlldeps binary_filename output_filename\n";

      printf( HelpText );
      return 1;
   }

   try
   {
      std::vector<std::string> dependencies;
      CollectDependentDlls(argv[1], dependencies);

      std::ofstream output(argv[2]);
      for(size_t i = 0; i < dependencies.size(); ++i)
         output << dependencies[i] << std::endl;

      output << std::endl;
   }
   catch(const std::exception& e)
   {
      std::cerr << e.what() << " GetLastError() returned #" << GetLastError() << std::endl;
      return 2;
   }

   return 0;
}
