//==================================
// PEDUMP - Matt Pietrek 1994-2001
// FILE: EXEDUMP.CPP
//==================================

#include "bindeps.h"
#include <exception>
#include <string>

#define IMAGE_SIZEOF_NT_OPTIONAL64_HEADER 240

// MakePtr is a macro that allows you to easily add to values (including
// pointers) together without dealing with C's pointer arithmetic.  It
// essentially treats the last two parameters as DWORDs.  The first
// parameter is used to typecast the result to the appropriate pointer type.
#define MakePtr( cast, ptr, addValue ) (cast)( (DWORD_PTR)(ptr) + (DWORD_PTR)(addValue))

#define GetImgDirEntryRVA( pNTHdr, IDE ) (pNTHdr->OptionalHeader.DataDirectory[IDE].VirtualAddress)

//================================================================================
//
// Given an RVA, look up the section header that encloses it and return a
// pointer to its IMAGE_SECTION_HEADER
//
template <class T> PIMAGE_SECTION_HEADER GetEnclosingSectionHeader(DWORD rva, T* pNTHeader)	// 'T' == PIMAGE_NT_HEADERS 
{
   PIMAGE_SECTION_HEADER section = IMAGE_FIRST_SECTION(pNTHeader);
   for ( size_t i = 0; i < pNTHeader->FileHeader.NumberOfSections; i++, section++ )
   {
      // This 3 line idiocy is because Watcom's linker actually sets the
      // Misc.VirtualSize field to 0.  (!!! - Retards....!!!)
      DWORD size = section->Misc.VirtualSize;
      if ( 0 == size )
         size = section->SizeOfRawData;

      // Is the RVA within this section?
      if ((section->VirtualAddress <= rva) && (rva < (section->VirtualAddress + size)))
         return section;
   }

   return 0;
}

template <class T> LPVOID GetPtrFromRVA( DWORD rva, T* pNTHeader, PBYTE imageBase ) // 'T' = PIMAGE_NT_HEADERS 
{
   PIMAGE_SECTION_HEADER pSectionHdr = GetEnclosingSectionHeader( rva, pNTHeader );
   if ( !pSectionHdr )
      return 0;

   INT delta = (INT)(pSectionHdr->VirtualAddress-pSectionHdr->PointerToRawData);
   return (PVOID) ( imageBase + rva - delta );
}

template <class T>	// 'T' = PIMAGE_NT_HEADERS 
void CollectDependentDlls(PBYTE pImageBase, T * pNTHeader, std::vector<std::string>& result)
{
   // Look up where the imports section is (normally in the .idata section)
   // but not necessarily so.  Therefore, grab the RVA from the data dir.
   DWORD importsStartRVA = GetImgDirEntryRVA(pNTHeader,IMAGE_DIRECTORY_ENTRY_IMPORT);
   if ( !importsStartRVA )
      return;

   PIMAGE_IMPORT_DESCRIPTOR pImportDesc = (PIMAGE_IMPORT_DESCRIPTOR)GetPtrFromRVA(importsStartRVA,pNTHeader,pImageBase);
   if (!pImportDesc)
      return;

   for( ; pImportDesc->TimeDateStamp || pImportDesc->Name; ++pImportDesc)
   {
      result.push_back((char*)GetPtrFromRVA(pImportDesc->Name, pNTHeader, pImageBase));
   }
}

void CollectDependentDlls( PIMAGE_DOS_HEADER dosHeader, std::vector<std::string>& result)
{
   PIMAGE_NT_HEADERS pNTHeader = MakePtr( PIMAGE_NT_HEADERS, dosHeader, dosHeader->e_lfanew );

   // First, verify that the e_lfanew field gave us a reasonable
   // pointer, then verify the PE signature.
   if (IsBadReadPtr(pNTHeader, sizeof(pNTHeader->Signature)) || (IMAGE_NT_SIGNATURE != pNTHeader->Signature))
      throw std::exception("Not a Portable Executable (PE) EXE");

   PBYTE pImageBase = (PBYTE)dosHeader;
   if ( pNTHeader->OptionalHeader.Magic == IMAGE_NT_OPTIONAL_HDR64_MAGIC )
   {
      PIMAGE_NT_HEADERS64 pNTHeader64 = (PIMAGE_NT_HEADERS64)pNTHeader;
      CollectDependentDlls(pImageBase, pNTHeader64, result);
   }
   else
   {
      CollectDependentDlls(pImageBase, pNTHeader, result);
   }
}
