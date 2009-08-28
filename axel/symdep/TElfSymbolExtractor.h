#include <elf.h>
#include "TFileIdent.h"

class TElfSymbolExtractorBase;

class TElfFileIdent: public TFileIdent {
public:
   TElfFileIdent(const char* name):
      TFileIdent(name, kNumTypes), fArch(0), fHeader(0), fExtractor(0) { 
      GetIdent();
   }
   virtual ~TElfFileIdent();

   void GetSymbols(TSymbolDependencyParser::DefSymMap_t& def,
                   std::list<const char*>& undef);

   UChar_t* Header() const { return fHeader; }

protected:
   TElfSymbolExtractorBase* SymbolExtractor();
   TElfSymbolExtractorBase* CreateSymbolExtractor();

private:
   void GetIdent();
   bool IsELF(UChar_t* e_ident) const;
   bool AssertDataLayout(UChar_t* e_ident) const;

   int fArch; // 32 or 64 bit
   UChar_t* fHeader; // elf header
   mutable TElfSymbolExtractorBase* fExtractor;
};

template <int IARCH>
class Elf_Arch {
public:
   typedef unsigned char UChar_t;
   typedef Elf64_Ehdr Ehdr;
   typedef Elf64_Shdr Shdr;
   typedef Elf64_Sym  Sym;
   typedef Elf64_Off  Off;
   static UChar_t st_bind(UChar_t info) { return ELF64_ST_BIND(info); }
   static UChar_t st_type(UChar_t info) { return ELF64_ST_TYPE(info); }
};
template<>
class Elf_Arch<32> {
public:
   typedef unsigned char UChar_t;
   typedef Elf32_Ehdr Ehdr;
   typedef Elf32_Shdr Shdr;
   typedef Elf32_Sym  Sym;
   typedef Elf32_Off  Off;
   static UChar_t st_bind(UChar_t info) { return ELF32_ST_BIND(info); }
   static UChar_t st_type(UChar_t info) { return ELF32_ST_TYPE(info); }
};


class TElfSymbolExtractorBase {
public:
   TElfSymbolExtractorBase(TFileIdent* id):
      fId(id), fFd(id->Fd()),
      fShNameBuf(0), fNameBuf(0) {}

   virtual ~TElfSymbolExtractorBase() {
      delete [] fShNameBuf;
      delete [] fNameBuf;
   }

   virtual void GetSymbols(TSymbolDependencyParser::DefSymMap_t& def,
                           std::list<const char*>& undef) = 0;

protected:
   void Fatal(const char* msg) const {
      fId->Fatal(msg);
   }

   TFileIdent* fId;
   FILE* fFd;
   char* fShNameBuf;
   char* fNameBuf;
};


template <int ARCH>
class TElfSymbolExtractorT: public TElfSymbolExtractorBase {
public:
   TElfSymbolExtractorT(TFileIdent* id):
      TElfSymbolExtractorBase(id),
      fDynSym(0), fSectionHeaders(0) {}

   virtual ~TElfSymbolExtractorT() {
      delete [] fSectionHeaders;
   }

   typedef Elf_Arch<ARCH> ElfArch;
   typedef typename ElfArch::Ehdr Ehdr;
   typedef typename ElfArch::Shdr Shdr;
   typedef typename ElfArch::Sym Sym;
   typedef typename ElfArch::Off Off;
   typedef std::list<Shdr*> ShdrList_t;

   void GetSymbols(TSymbolDependencyParser::DefSymMap_t& def,
                   std::list<const char*>& undef);
private:
   void GetSections();
   char* ExtractNames(Shdr* sect);

   Shdr* fDynSym;
   Shdr* fSectionHeaders;
};

template class TElfSymbolExtractorT<32>;
template class TElfSymbolExtractorT<64>;
