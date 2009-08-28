#include "TElfSymbolExtractor.h"

#include "elf.h"
#include <iostream>
#include <cstring>
#include <map>

TElfFileIdent::~TElfFileIdent() {
   delete [] fHeader;
   delete fExtractor;
}

// We assume the the file is LSB if on a LSB platform,
// or MSB if on a MSB platform, i.e. that we are not
// investigating an alien file.

void TElfFileIdent::GetIdent() {
   // Initialize the internal structures,
   // return open FILE*
   UChar_t e_ident[EI_NIDENT];
   if (fseek(fFd, 0, SEEK_SET)) {
      Fatal("cannot be seeked in!");
   }
   size_t nread = fread(e_ident, sizeof(e_ident), 1, fFd);
   if (nread != 1) {
      Fatal("is too short or unreadable!");
   }

   if (!IsELF(e_ident)) {
      Fatal("is not an ELF file!");
   }
   AssertDataLayout(e_ident);

   if (e_ident[EI_CLASS] == ELFCLASS32) {
      fArch = 32;
   } else if (e_ident[EI_CLASS] == ELFCLASS64) {
      fArch = 64;
   } else {
      Fatal("has an invalid architecture!");
   }

   size_t lenheader = 0;
   if (fArch == 32) {
      lenheader = sizeof(Elf32_Ehdr);
   } else {
      lenheader = sizeof(Elf64_Ehdr);
   }
   fHeader = new UChar_t[lenheader];

   if (fseek(fFd, 0, SEEK_SET)) {
      Fatal("cannot be seeked in!");
   }
   nread = fread(fHeader, lenheader, 1, fFd);
   if (nread != 1) {
      Fatal("Cannot read ELF header, file corrupted?");
   }
   uint16_t e_type = 0;
   if (fArch == 32) {
      e_type = ((Elf32_Ehdr*)fHeader)->e_type;
   } else {
      e_type = ((Elf64_Ehdr*)fHeader)->e_type;
   }

   switch (e_type) {
   case ET_REL: fType = TFileIdent::kObject; break;
   case ET_DYN: fType = TFileIdent::kSharedLib; break;
   default: fType = TFileIdent::kOther;
   }
   if (fType == TFileIdent::kOther) {
      Fatal("has an unhandled format (neither shared library nor object file)");
   }
}

bool TElfFileIdent::IsELF(UChar_t* e_ident) const {
   return (e_ident[EI_MAG0] == ELFMAG0)
      && (e_ident[EI_MAG1] == ELFMAG1)
      && (e_ident[EI_MAG2] == ELFMAG2)
      && (e_ident[EI_MAG3] == ELFMAG3);
}

bool TElfFileIdent::AssertDataLayout(UChar_t* e_ident) const {
   if (IsLSB()) {
      if (e_ident[EI_DATA] != ELFDATA2LSB) {
         Fatal("does not correspond to system data layout (lsb)");
      }
   } else {
      if (e_ident[EI_DATA] != ELFDATA2MSB) {
         Fatal("does not correspond to system data layout (msb)");
      }
   }
   return true;
}

void TElfFileIdent::GetSymbols(TSymbolDependencyParser::DefSymMap_t& def,
                               std::list<const char*>& undef) {
   SymbolExtractor()->GetSymbols(def, undef);
   fclose(fFd);
   fFd = 0;
}

TElfSymbolExtractorBase* TElfFileIdent::SymbolExtractor() {
   if (!fExtractor) {
      fExtractor = CreateSymbolExtractor();
   }
   return fExtractor;
}

TElfSymbolExtractorBase* TElfFileIdent::CreateSymbolExtractor() {
   if (fArch == 32) {
      return new TElfSymbolExtractorT<32>(this);
   }
   return new TElfSymbolExtractorT<64>(this);
}




template <int ARCH>
void TElfSymbolExtractorT<ARCH>::GetSymbols(TSymbolDependencyParser::DefSymMap_t& def,
                                            std::list<const char*>& undef) {
   // print defined symbols for shared libs,
   // undefined symbols for object files.
   if (!fSectionHeaders) {
      GetSections();
   }
   bool isObj = (fId->Type() != TFileIdent::kSharedLib);
   /*
   std::cout << std::endl << std::endl << "FILE: " << fId->Name();
   if (isObj) 
      std::cout << " [obj]" << std::endl;
   else
      std::cout << " [so]" << std::endl;
   */
   fseek(fFd, fDynSym->sh_offset, SEEK_SET);
   size_t sh_entsize = fDynSym->sh_entsize;
   size_t sh_num = fDynSym->sh_size / sh_entsize;
   Sym* syms = new Sym[sh_num];
   fread(syms, fDynSym->sh_size, 1, fFd);
   for (size_t s = 0; s < sh_num; ++s) {
      bool isdefined = syms[s].st_shndx != SHN_UNDEF;
      const char* name = fNameBuf + syms[s].st_name;
      if (name[0] == 0) continue;
      if (isObj) {
         if (!isdefined) {
            undef.push_back(name);
            // std::cout << "  U " << name << std::endl;
         }
         // we don't care about defined symbols in object files:
         // they will be linked anyway.
      } else {
         // lib
         if (isdefined) {
            if (ElfArch::st_type(syms[s].st_info) == STT_FUNC
                && ElfArch::st_bind(syms[s].st_info) == STB_GLOBAL) {
               def[name] = fId->Name();
               // std::cout << "  T " << name << std::endl;
            }
         } else {
            undef.push_back(name);
            // std::cout << "  U " << name << std::endl;
         }
      }
   }
   delete []syms;
}

template <int ARCH>
void TElfSymbolExtractorT<ARCH>::GetSections() {
   TElfFileIdent* elfId = (TElfFileIdent*)fId;
   Ehdr* hdr = (Ehdr*)elfId->Header();
   const size_t entsize = hdr->e_shentsize;
   const size_t shnum = hdr->e_shnum;
   fseek(fFd, hdr->e_shoff, SEEK_SET);
   fSectionHeaders = new Shdr[shnum];
   if (fread(fSectionHeaders, entsize*shnum, 1, fFd) != 1) {
      Fatal("cannot read ELF section headers!");
   }

   Shdr* shStrTab = &fSectionHeaders[hdr->e_shstrndx];
   fShNameBuf = ExtractNames(shStrTab);

   uint32_t want_type = SHT_DYNSYM;
   const char* want_sectname = ".dynstr";
   if (fId->Type() != TFileIdent::kSharedLib) {
      want_type = SHT_SYMTAB;
      want_sectname = ".strtab";
   }

   for (size_t s = 0; s < shnum; ++s) {
      Shdr* sect = &fSectionHeaders[s];
      const char* sectName = fShNameBuf + sect->sh_name;
      //std::cerr << sectName << std::endl;
      if (sect->sh_type == want_type) {
         fDynSym = sect;
      }
      else if (sect->sh_type == SHT_STRTAB) {
         if (!strcmp(sectName, want_sectname)) {
            //std::cerr << "found dynstr!" <<std::endl;
            fNameBuf = ExtractNames(sect);
         }
      }
      else continue;
   }
}


template <int ARCH>
char* TElfSymbolExtractorT<ARCH>::ExtractNames(Shdr* sect) {
   size_t lensect = sect->sh_size;
   char* buf = new char[lensect];
         
   fseek(fFd, sect->sh_offset, SEEK_SET);
   if (fread(buf, lensect, 1, fFd) != 1) {
      Fatal("does not contain readable strtab!");
   }
   return buf;
}
