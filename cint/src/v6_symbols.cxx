/* /% C %/ */
/***********************************************************************
 * cint (C/C++ interpreter)
 ************************************************************************
 * Source file symbols.cxx
 * Leandro Franco
 ************************************************************************
 * Description:
 *  This file performs the reading and registering of library symbols.
 *  This is done creating the dictionaries and the idea is to add a new 
 *  field to the memfunc setup containing the mangled name for the symbol
 ************************************************************************
 * Copyright(c) 1995~2004  Masaharu Goto
 *
 * For the licensing terms see the file COPYING
 *
 ************************************************************************/

#include "common.h"
#include "Api.h"

#include <cxxabi.h>
#include <dlfcn.h>
#include <link.h>
#include <iostream>
#include <fstream>

#include <vector>
#include <list>
#include <set>
#include <string>
#include <map>
using namespace std;

static int gDebug = 0;

static unsigned long hash(const void *txt, int ntxt)
{
   // Calculates hash index from any char string. (static function)
   // Based on precalculated table of 256 specially selected numbers.
   // These numbers are selected in such a way, that for string
   // length == 4 (integer number) the hash is unambigous, i.e.
   // from hash value we can recalculate input (no degeneration).
   //
   // The quality of hash method is good enough, that
   // "random" numbers made as R = Hash(1), Hash(2), ...Hash(N)
   // tested by <R>, <R*R>, <Ri*Ri+1> gives the same result
   // as for libc rand().
   //
   // For string:  i = TString::Hash(string,nstring);
   // For int:     i = TString::Hash(&intword,sizeof(int));
   // For pointer: i = TString::Hash(&pointer,sizeof(void*));
   //
   //              V.Perev

   static const long utab[] = {
      0xdd367647,0x9caf993f,0x3f3cc5ff,0xfde25082,0x4c764b21,0x89affca7,0x5431965c,0xce22eeec,
      0xc61ab4dc,0x59cc93bd,0xed3107e3,0x0b0a287a,0x4712475a,0xce4a4c71,0x352c8403,0x94cb3cee,
      0xc3ac509b,0x09f827a2,0xce02e37e,0x7b20bbba,0x76adcedc,0x18c52663,0x19f74103,0x6f30e47b,
      0x132ea5a1,0xfdd279e0,0xa3d57d00,0xcff9cb40,0x9617f384,0x6411acfa,0xff908678,0x5c796b2c,
      0x4471b62d,0xd38e3275,0xdb57912d,0x26bf953f,0xfc41b2a5,0xe64bcebd,0x190b7839,0x7e8e6a56,
      0x9ca22311,0xef28aa60,0xe6b9208e,0xd257fb65,0x45781c2c,0x9a558ac3,0x2743e74d,0x839417a8,
      0x06b54d5d,0x1a82bcb4,0x06e97a66,0x70abdd03,0xd163f30d,0x222ed322,0x777bfeda,0xab7a2e83,
      0x8494e0cf,0x2dca2d4f,0x78f94278,0x33f04a09,0x402b6452,0x0cd8b709,0xdb72a39e,0x170e00a2,
      0x26354faa,0x80e57453,0xcfe8d4e1,0x19e45254,0x04c291c3,0xeb503738,0x425af3bc,0x67836f2a,
      0xfac22add,0xfafc2b8c,0x59b8c2a0,0x03e806f9,0xcb4938b9,0xccc942af,0xcee3ae2e,0xfbe748fa,
      0xb223a075,0x85c49b5d,0xe4576ac9,0x0fbd46e2,0xb49f9cf5,0xf3e1e86a,0x7d7927fb,0x711afe12,
      0xbf61c346,0x157c9956,0x86b6b046,0x2e402146,0xb2a57d8a,0x0d064bb1,0x30ce390c,0x3a3e1eb1,
      0xbe7f6f8f,0xd8e30f87,0x5be2813c,0x73a3a901,0xa3aaf967,0x59ff092c,0x1705c798,0xf610dd66,
      0xb17da91e,0x8e59534e,0x2211ea5b,0xa804ba03,0xd890efbb,0xb8b48110,0xff390068,0xc8c325b4,
      0xf7289c07,0x787e104f,0x3d0df3d0,0x3526796d,0x10548055,0x1d59a42b,0xed1cc5a3,0xdd45372a,
      0x31c50d57,0x65757cb7,0x3cfb85be,0xa329910d,0x6ad8ce39,0xa2de44de,0x0dd32432,0xd4a5b617,
      0x8f3107fc,0x96485175,0x7f94d4f3,0x35097634,0xdb3ca782,0x2c0290b8,0x2045300b,0xe0f5d15a,
      0x0e8cbffa,0xaa1cc38a,0x84008d6f,0xe9a9e794,0x5c602c25,0xfa3658fa,0x98d9d82b,0x3f1497e7,
      0x84b6f031,0xe381eff9,0xfc7ae252,0xb239e05d,0xe3723d1f,0xcc3bda82,0xe21b1ad3,0x9104f7c8,
      0x4bb2dfcd,0x4d14a8bc,0x6ba7f28c,0x8f89886c,0xad44c97e,0xb30fd975,0x633cdab1,0xf6c2d514,
      0x067a49d2,0xdc461ad9,0xebaf9f3f,0x8dc6cac3,0x7a060f16,0xbab063ad,0xf42e25e6,0x60724ca6,
      0xc7245c2e,0x4e48ea3c,0x9f89a609,0xa1c49890,0x4bb7f116,0xd722865c,0xa8ee3995,0x0ee070b1,
      0xd9bffcc2,0xe55b64f9,0x25507a5a,0xc7a3e2b5,0x5f395f7e,0xe7957652,0x7381ba6a,0xde3d21f1,
      0xdf1708dd,0xad0c9d0c,0x00cbc9e5,0x1160e833,0x6779582c,0x29d5d393,0x3f11d7d7,0x826a6b9b,
      0xe73ff12f,0x8bad3d86,0xee41d3e5,0x7f0c8917,0x8089ef24,0x90c5cb28,0x2f7f8e6b,0x6966418a,
      0x345453fb,0x7a2f8a68,0xf198593d,0xc079a532,0xc1971e81,0x1ab74e26,0x329ef347,0x7423d3d0,
      0x942c510b,0x7f6c6382,0x14ae6acc,0x64b59da7,0x2356fa47,0xb6749d9c,0x499de1bb,0x92ffd191,
      0xe8f2fb75,0x848dc913,0x3e8727d3,0x1dcffe61,0xb6e45245,0x49055738,0x827a6b55,0xb4788887,
      0x7e680125,0xd19ce7ed,0x6b4b8e30,0xa8cadea2,0x216035d8,0x1c63bc3c,0xe1299056,0x1ad3dff4,
      0x0aefd13c,0x0e7b921c,0xca0173c6,0x9995782d,0xcccfd494,0xd4b0ac88,0x53d552b1,0x630dae8b,
      0xa8332dad,0x7139d9a2,0x5d76f2c4,0x7a4f8f1e,0x8d1aef97,0xd1cf285d,0xc8239153,0xce2608a9,
      0x7b562475,0xe4b4bc83,0xf3db0c3a,0x70a65e48,0x6016b302,0xdebd5046,0x707e786a,0x6f10200c
   };

   static const long msk[] = { 0x11111111, 0x33333333, 0x77777777, 0xffffffff };

   const unsigned char *uc = (const unsigned char *) txt;
   long uu = 0;
   union {
      long  u;
      short s[2];
   } u;
   u.u = 0;
   int i, idx;

   for (i = 0; i < ntxt; i++) {
      idx = (uc[i] ^ i) & 255;
      uu  = (uu << 1) ^ (utab[idx] & msk[i & 3]);
      if ((i & 3) == 3) u.u ^= uu;
   }
   if (i & 3) u.u ^= uu;

   u.u *= 1879048201;      // prime number
   u.s[0] += u.s[1];
   u.u *= 1979048191;      // prime number
   u.s[1] ^= u.s[0];
   u.u *= 2079048197;      // prime number

   return u.u;
}


struct ptr_delete {
  template< class T >
  void operator()( T * const p ) const {
    delete p;
  }
};

// This is a simple class that contains the information
// needed for a symbol. In particular, its name and address
class TSymbol {
private:
   TSymbol(const TSymbol& sym);       //Avoid copying ptrs
   TSymbol& operator=(const TSymbol& sym);  //Avoid copying ptrs

public:
   void     *fFunc;
   //unsigned int    fHash;
   unsigned int    fClassHash;

   // This points directly to the name in the library
   string  fMangled;
      
   bool    fIsDest;
   bool    fIsDestInC;
   bool    fIsDestInCDel;
   bool    fIsConst;
   bool    fIsConstInC;

   TSymbol()
      : fFunc(0), /*fHash(0),*/ fClassHash(0), fIsDest(false), fIsDestInC(false), 
        fIsDestInCDel(false), fIsConst(false), fIsConstInC(false)
   {
   }

   TSymbol(string &mangled, void* func)
   : fFunc(func), /*fHash(0),*/ fClassHash(0), fMangled(mangled), fIsDest(false), 
     fIsDestInC(false), fIsDestInCDel(false), fIsConst(false), fIsConstInC(false)
   {
   }

   ~TSymbol()
   {
   }

   void SetHash(char *demanstr)
   {
      if(!demanstr)
         return;

      string demangled(demanstr);

      // First we have to tell it that we are only interested for
      // the strings that are _before_ the ()
      string::size_type ind =  demangled.find("(");

      // maybe this symbol is not a function
      if (ind == string::npos)
         return;

      //TString substr = demangled->operator()(0,ind);
      string::size_type start = 0;
      string::size_type end   = demangled.find(" ", start);
      
      while(start != string::npos) {
         if ((end == string::npos) || (end > ind))
            end = ind;
         
         int ncolon=0;
         string::size_type cstart = start;
         string::size_type cidx = start;
         while((cidx != string::npos) && (cidx < ind)){
            cidx = demangled.find("::", cstart);

            if((cidx != string::npos) && (cidx < ind)){
               ++ncolon;
               cstart = cidx+2;
            }
         }
         
         // if(ncolon>1)
         //   cout << "demanstr: " << demanstr << " ncolon: " << ncolon << endl;

         // LF 15-10-07
         // Be careful with namespaces...
         // things like TBits::TReference::~TReference()
         // will confuse our naive algorithm... instead of just looking
         // for '::', look for the last pair of '::'
         string::size_type icolon = string::npos;
         string::size_type istart = start;
         for (int i=0;i<ncolon;i++){
            if(icolon != string::npos)
               start = istart;

            icolon = demangled.find("::", istart);
            istart = icolon+2;
         }
         //string::size_type icolon = demangled.find("::", start);
         
         if (icolon != string::npos && icolon < ind){
            // Now hash the class name also
            // (it will make things easier when we try to register
            // symbols by class)
            //string classname = string(demangled, start, icolon);
            string classname = demangled.substr(0, icolon);
            string classname_noname = demangled.substr(start, icolon-start);
            string protoname = demangled.substr(icolon+2, ind - (icolon+2));


            // 11-10-07
            // Get rid of the "<>" part in something like TParameter<float>::TParameter()
            string::size_type itri = classname.find("<");

            string classname_notemp;
            if(itri != string::npos){
               classname_notemp = classname_noname.substr(0, itri);
            }
            else
               classname_notemp = classname_noname;

            // ** constructors
            if ( classname_notemp == protoname) {
               fIsConst = true;
               // if this not the constructor in charge then just continue
               //string mang(classname_notemp);
               //mang += "C1";
               
               //TString sub = fMangled->SubString(mang);
               if(!(strstr(fMangled.c_str(), classname_notemp.c_str()) && strstr(fMangled.c_str(), "C1")  )) {
                  //if (sub.IsNull()) {
                  // This is not the constructor in-charge... ignore it
                  return;
               }
               fIsConstInC = true;
            }

            // ** destructors
            string dest("~");
            dest += classname_notemp;
            if ( dest == protoname){
               fIsDest = true;
               // if this not the constructor in charge then just continue
               //string mang0(classname);
               //string mang1(classname);
               //mang0 += "D0";
               //mang1 += "D1";
               
               //if (!(fMangled->SubString(mang0)).IsNull()) {
               if(strstr(fMangled.c_str(), classname_notemp.c_str()) && strstr(fMangled.c_str(), "D0")) {
               // This is the deleting constructor
                  fIsDestInCDel = true;

                  // LF 27-07-07
                  // We ignore the deleting constructor since we do
                  // that job in newlink
                  return;
               }
               //else if(!(fMangled->SubString(mang1)).IsNull()){
               else if( strstr(fMangled.c_str(), classname_notemp.c_str()) && strstr(fMangled.c_str(), "D1") ){
                  // This is the in-charge (non deleting) constructor
                  fIsDestInC = true;
                  
                  // If the in-charge deleting destructor was already
                  // registered then dont register the in-charge (non deleting)
                  // destructor
                  //if(destInChargeDel)
                  //   continue;
               }
               else {
                  // Ignoring destructor not-in-charge for" << classstr.Data() << endl; 
                  return;
               }
            }

            //fHash = demangled.Hash();
            //this->SetName(demangled.Data());

            // LF 05-11-07
            // We can get something like:
            // ROOT::Math::SVector<double, 2u>
            // When we have actually declared
            // ROOT::Math::SVector<double,2>
            // in ROOT... so the hashes wont match..
            // try to convert the former in the latter.
            
            string::size_type pos_smaller  = classname.find_first_of("<", 0);
            string::size_type pos_greater  = classname.find_last_of(">", classname.size()-1);

            string::size_type index=0;
            while(index < classname.size()){
               if(classname[index]==' ' && 
                  (index>0 && !isalpha(classname[index-1])) && 
                  (index<classname.size()-1 && !isalpha(classname[index+1])) ){
                  classname.erase(index,1);
                  pos_greater  = classname.find_last_of(">", classname.size()-1);
               }
               else if( (pos_smaller != string::npos) &&
                   (pos_greater != string::npos) &&
                   index>pos_smaller && index<pos_greater ) {
                  // How is the isinteger(char) when using the stl?
                  if( (classname[index]=='0' || classname[index]=='1' || classname[index]=='2' || classname[index]=='3' || classname[index]=='4' ||
                       classname[index]=='5' || classname[index]=='6' || classname[index]=='7' || classname[index]=='8' || classname[index]=='9')
                      && classname[index+1]=='u'){
                     classname.erase(index+1,1);
                     pos_greater  = classname.find_last_of(">", classname.size()-1);
                  }
                  else
                     index++;
               }
               else
                  index++;
            }
            
            fClassHash = hash(classname.c_str(), classname.size());
            return;
         }
         else {
            fClassHash = hash(demanstr, strlen(demanstr));
            //fHash = demangled.Hash();
            //this->SetName(demangled.Data());
            return;
         }

         start = demangled.find(" ", start);
         if(start != string::npos)
            end = demangled.find(" ", start);
      }
   }

   unsigned long Hash() const
   {
      // The hash of a symbol is going to be hash of the method's name
      // ie. "TObject::Dump" for the method dump in TObject

      if (fClassHash)
         return fClassHash;

      return 0;
   }

   char* Demangle()
   {
      // demangles the string pointed by mangled and returns 
      // a new char* containing the demangled name
      // rem to free that value

      //if(!fMangled) {
      //   cerr << "Fatal error: Demangle() fMangled is NULL"<< endl;
      //   return 0;
      //}

      // LF: 22/05/07
      // Instead of copying the fMangled name just do the demangling
      // I dont know why the symbols start with __Z if the ABI wants them
      // to start with _Z... until understanding it just hack your way through
      const char *cline = fMangled.c_str();
      if ( cline[0]=='_' && cline[1]=='_') {
         cline = cline++;
      }
      
      // We have the symbols string in "line"
      // now try to demangle it
      int status = 0;
      char *name = abi::__cxa_demangle(cline, 0, 0, &status);
      if (!name)
      {
         // It probably wasnt a valid symbol...
         // to be sure check the status code
         // "error code = 0: success";
         // "error code = -1: memory allocation failure";
         // "error code = -2: invalid fMangled name";
         // "error code = -3: invalid arguments";
         return 0;
      }
      
      // Ignore this symbols if it was generated by cint
      if( !strncmp( name, "G__", 3) || strstr(name, "thunk")){
         if (name)
            free(name);
         
         return 0;
      }
      
      return name;
   }
   
   //ClassDef(TSymbol,0)

};


// This will be the lookup for the symbols. The idea is to have
// a list of the available (open) libraries with links to 
// its symbols and the classes that have already been registered
class TSymbolLookup {

private:
   TSymbolLookup(const TSymbolLookup& sym);             //Avoid copying ptrs
   TSymbolLookup& operator=(const TSymbolLookup& sym);  //Avoid copying ptrs

public:
   string    fLibname;
   //TObjArray  *fRegistered;
   std::list<std::string>  fRegistered;
   std::list<TSymbol*>    *fSymbols;

   TSymbolLookup()
   {
      //fLibname    = 0;
      //fRegistered = 0;
      fSymbols    = 0;
   }

   TSymbolLookup(const char *libname, std::list<TSymbol*> *symbols)
   {
      //fRegistered = 0;
      fSymbols    = 0;
      fLibname    = string(libname);
      fSymbols    = symbols;

      //this->SetName(libname);
      
      //if(fSymbols)
      //   fSymbols->SetOwner();
   }

   ~TSymbolLookup()
   {
      //delete fLibname;
      //delete fRegistered;
      //delete fSymbols;
      std::for_each( fSymbols->begin(), fSymbols->end(), ptr_delete() );
      fSymbols->clear();
   }
   
   void AddRegistered(string &classname)
   {
      // This will add a new classname to the list of registered classes
      //if(!fRegistered) {
      //   fRegistered = new TObjArray();
      //   fRegistered->SetOwner();
      //}

      //fRegistered->Add((TObject*)new TObjString(classname.c_str()));
      fRegistered.push_back(classname);
   }

   unsigned long Hash() const
   {
      // This is just the name of the library
      // (always remember that this is the fullpath)

      if (fLibname.size())
         return hash(fLibname.c_str(), fLibname.size());

      return 0;
   }

   bool IsRegistered(string &classname)
   {
      // Let's check if this class has already been registered
      std::list<string>::iterator iter = find(fRegistered.begin(), fRegistered.end(), classname);
      if (iter != fRegistered.end())
         return true;

      return false;
   }
   
   void SetLibname(string &libname)
   {
      // Give a libname to this object
      //delete fLibname;

      fLibname = string(libname);
      //this->SetName(fLibname.c_str());
   }

   void SetSymbols(std::list<TSymbol*> *table)
   {
      // Associate a symbol table to this library
      //delete fSymbols;
      std::for_each( fSymbols->begin(), fSymbols->end(), ptr_delete() );
      fSymbols->clear();

      fSymbols = table;
      //fSymbols->SetOwner();
   }

};


std::multimap<std::string, TSymbolLookup*>*
G__get_cache()
{
   // sort of singleton for the symbol cache

   static multimap<string, TSymbolLookup*> G__cache;
   return &G__cache;
}


//______________________________________________________________________________
//______________________________________________________________________________
// Methods used to register the function pointer (or method pointer)
// of a given signature inside CINT. This should probably go to another file
//
//______________________________________________________________________________
//______________________________________________________________________________

//______________________________________________________________________________
void MapDependantTypes()
{
    int more = 0;
    char prompt[G__ONELINE];

   // There is problem with types ( well many :) ).
   // We can declare something like "FILE *fp" in our
   // normal ROOT classes but when gcc creates the symbols
   // it will use an internal type like "_IO_FILE".
   // This mapping is completely dependant of the compiler
   // so we have to be very careful about it.
   //ProcessLine("typedef FILE _IO_FILE", 0);
   G__process_cmd ("typedef FILE _IO_FILE", prompt, &more,0, 0);

   // gui/inc/WidgetMessageTypes.h
   // All the enums could be treated as ints
   //ProcessLine("typedef int EWidgetMessageTypes", 0);
   G__process_cmd ("typedef int EWidgetMessageTypes", prompt, &more,0, 0);

   //ProcessLine("typedef void* istream", 0);
   G__process_cmd ("typedef void* istream", prompt, &more,0, 0);

   //ProcessLine("typedef void* istream&", 0);
   G__process_cmd ("typedef void* istream&", prompt, &more,0, 0);

   //ProcessLine("typedef void* std::istream&", 0);
   G__process_cmd ("typedef void* std::istream&", prompt, &more,0, 0);

   //ProcessLine("typedef int __false_type", 0);
   G__process_cmd ("typedef int __false_type", prompt, &more,0, 0);

   //ProcessLine("typedef int __true_type", 0);
   G__process_cmd ("typedef int __true_type", prompt, &more,0, 0);


}

//______________________________________________________________________________
void MapDependantTypesX()
{
   int more = 0;
   char prompt[G__ONELINE];

   // Like MapDependantTypes but for the class TGX11

   //ProcessLine("typedef struct _XDisplay   { } Display;", 0);
   G__process_cmd("typedef struct _XDisplay  { } Display;", prompt, &more,0, 0);
   
   //ProcessLine("typedef struct _XGC        { } *GC;", 0);
   G__process_cmd("typedef struct _XGC       { } *GC;", prompt, &more,0, 0);

   //ProcessLine("typedef struct _XEvent     { } XEvent", 0);
   G__process_cmd("typedef struct _XEvent    { } XEvent", prompt, &more,0, 0);

   //ProcessLine("typedef struct _XImage     { } Image", 0);
   G__process_cmd("typedef struct _XImage    { } Image", prompt, &more,0, 0);

   //ProcessLine("typedef struct             { } XFontStruct;", 0);
   G__process_cmd("typedef struct            { } XFontStruct;", prompt, &more,0, 0);

   //ProcessLine("typedef struct FT_Bitmap_  { } FT_Bitmap", 0);
   G__process_cmd("typedef struct FT_Bitmap_ { } FT_Bitmap", prompt, &more,0, 0);
}

//______________________________________________________________________________
void MapDependantTypesTree()
{

   int more = 0;
   char prompt[G__ONELINE];

   // dependant type in libTree.so
   
   // LF 07/05/2007
   // This class doesnt have a classdef so I dont know yet how to handle it
   //ProcessLine("typedef struct {} TTree::TFriendLock", 0);
   //ProcessLine("typedef struct {} TFriendLock", 0);
   G__process_cmd("typedef struct {} TFriendLock", prompt, &more,0, 0);

   //ProcessLine("typedef ifstream basic_ifstream<char, std::char_traits<char>>", 0);
   G__process_cmd("typedef ifstream basic_ifstream<char, std::char_traits<char>>", prompt, &more,0, 0);
}

//______________________________________________________________________________
struct link_map *
locate_linkmap()
{
   /* locate base link-map in memory */
   //
   // LF: 08/05/07
   // Taken from the phrack magazine but modified to find the link map
   // of the current process (as such we dont need ptracing abilities)

   Elf32_Ehdr      *ehdr   = (Elf32_Ehdr*) malloc(sizeof(Elf32_Ehdr));
   Elf32_Phdr      *phdr   = (Elf32_Phdr*) malloc(sizeof(Elf32_Phdr));
   Elf32_Dyn       *dyn    = (Elf32_Dyn*)  malloc(sizeof(Elf32_Dyn));
   Elf32_Word      got;
   struct link_map *l      = (link_map*)   malloc(sizeof(struct link_map));
   unsigned long   phdr_addr , dyn_addr , map_addr;


   /* first we check from elf header, mapped at 0x08048000, the offset
    * to the program header table from where we try to locate
    * PT_DYNAMIC section.
    */
   memcpy(ehdr, (const void*)0x08048000, sizeof(Elf32_Ehdr));

   phdr_addr = 0x08048000 + ehdr->e_phoff;

   memcpy(phdr, (const void*)phdr_addr, sizeof(Elf32_Phdr));

   while ( phdr->p_type != PT_DYNAMIC ) {
      memcpy(phdr, (const void*) (phdr_addr += sizeof(Elf32_Phdr)), sizeof(Elf32_Phdr));
   }

   /* now go through dynamic section until we find address of the GOT
    */

   memcpy(dyn, (const void*)phdr->p_vaddr, sizeof(Elf32_Dyn));
   dyn_addr = phdr->p_vaddr;

   while ( dyn->d_tag != DT_PLTGOT ) {
      memcpy(dyn, (const void*)(dyn_addr += sizeof(Elf32_Dyn)), sizeof(Elf32_Dyn));
   }

   got = (Elf32_Word) dyn->d_un.d_ptr;
   got += 4; 		/* second GOT entry, remember? */

   /* now just read first link_map item and return it */
   memcpy(&map_addr, (const void*) got, 4);
   
   memcpy(l, (const void*)map_addr, sizeof(struct link_map));

   free(phdr);
   free(ehdr);
   free(dyn);

   return l;
}

//______________________________________________________________________________
struct link_map *
get_linkmap(const char *libname)
{
   // Get the list of the libraries currently loaded and compare it to 
   // "libname". If it's found return the link_map for that library
   
   //struct link_map *map = locate_linkmap();
   //struct link_map *map_iter;
   //struct link_map *map_res = 0;

   //for (map_iter = map; map_iter; map_iter = map_iter->l_next) {
   //   if( map_iter->l_name && 
   //       (strcmp(map_iter->l_name, libname)==0 ||
   //        strcmp(gSystem->BaseName(map_iter->l_name), gSystem->BaseName(libname))==0 )) {
   //      map_res = map_iter;
   //      break;
   //   }
   //}

   //free(map);
   //return map_res;

   // LF 04-07-07 dont scan the memory... just open the library
   void* handle = dlopen(libname, RTLD_LAZY);
   if (!handle) {
      cerr << "Error during dlopen(): " << dlerror() << endl;
      return 0;
   }

   struct link_map *lmap;
   if (dlinfo (handle, RTLD_DI_LINKMAP, &lmap) == -1)
      cerr << "Error during dlinfo(): " << dlerror() << endl;
   
   return lmap;
}

/*
//______________________________________________________________________________
static std::list<TSymbol*>*
// static TList*
SymbolList(const char* lib)
{
   // Read the exported symbols from a shared library
   // by exploring the Symbol Hash Table
   // be careful with this function, "symbols" and "address"
   // will be allocated here so it's up to the user to
   // free that stuff from the heap
   // Note:  it's static because we dont want to export the symbol

   struct link_map *map = get_linkmap(lib);
   if(!map) {
      cerr << "Fatal Error: This library has not been loaded yet: " << lib << endl;
      return 0;
   }

   // I have to use a new HashList here since the hash changes...
   // before it was zero and now it's teh real hash... it has
   // to be modified for something more practical!!!
   std::list<TSymbol*> *symbol_list = new std::list<TSymbol*>;
   //TList *symbol_list = new TList();
   //   symbol_list->SetOwner();

   // Strings Table accessing 
   // The symbol's names are in a separate table 
   char *strtab_pointer;

   for (Elf32_Dyn* dyn = map->l_ld; dyn->d_tag != DT_NULL; ++dyn)
      if (dyn->d_tag == DT_STRTAB)
         strtab_pointer = (char *) dyn->d_un.d_ptr;

   // Symbol Table Entries Accessing 
   // Symbols in ELF file are in the Table Entrie Structure 
   Elf32_Sym *symtab_pointer;
   Elf32_Word *hashtab_pointer;
   int symtab_size;
   int strtab_size;

   Elf32_Addr lib_base_addr = map->l_addr;

   for (Elf32_Dyn* dyn =  map->l_ld ; dyn->d_tag != DT_NULL; ++dyn)
   {
      if (dyn->d_tag == DT_SYMTAB)
         symtab_pointer = (Elf32_Sym *) dyn->d_un.d_ptr;
      if (dyn->d_tag ==  DT_SYMENT)
         symtab_size = dyn->d_un.d_val;
      if (dyn->d_tag ==  DT_STRSZ)
         strtab_size = dyn->d_un.d_val;
      if (dyn->d_tag ==  DT_HASH)
         hashtab_pointer = (Elf32_Word *) dyn->d_un.d_ptr;
   }

// Hash table accessing 
// Finally we search the symbols in the Hash table
// The Hash table is a list of pointers to Symbol Entries structures 
   int nbucket = hashtab_pointer[0];
   int nchain = hashtab_pointer[1];

   // Total Number of characters
   //int symbols_chain_size = 0;
   // Total Number of symbols
   //int symbols_number = 0;

   // Looking for the number of symbols and names size in the Hash Table
   //for (int j = 2; j < nbucket + nchain; j++)
   //{
   //   Elf32_Sym sym_entry = symtab_pointer[hashtab_pointer[j]];

   //   if (sym_entry.st_name != 0){
   //      for (int j = sym_entry.st_name; (j < strtab_size) && (strtab_pointer[j] != '\0'); j++){
   //         symbols_chain_size++;
   //      }
   //      // End of symbol
   //      symbols_chain_size++;
   //      symbols_number++;
   //   }
   //}
   // Temporary Array for Listing Symbols
   //char* temp = new char[symbols_chain_size+1];
   //int * tempAddress = new int[symbols_number];

   // Looking for the symbols names in the Name List
   for (int i = 2; i < nbucket + nchain; i++)
   {
      Elf32_Sym sym_entry = symtab_pointer[hashtab_pointer[i]];

      if (sym_entry.st_name != 0){
         // If symbol not defined in the library. It should be in the dependencies. It return 0 like Address
         if ((sym_entry.st_value == 0) || (ELF32_ST_TYPE(sym_entry.st_info) != STT_FUNC))
            continue;

         //tempAddress[addIndex] = (int) sym_entry.st_value + (int) lib_base_addr;
         //addIndex++;

         // Copy the mangled name
         char *mangle_name = sym_entry.st_name + strtab_pointer;

         // LF: 22/05/07
         // Instead of copying the mangled name just do the demangling
         // I dont know why the symbols start with __Z if the ABI wants them
         // to start with _Z... until understanding it just hack your way through
         //const char *cline = mangle_name;
         //if ( cline[0]=='_' && cline[1]=='_') {
         //   cline = cline++;
         //}

         // We have the symbols string in "line"
         // now try to demangle it
         //int status = 0;
         //char *name = abi::__cxa_demangle(cline, 0, 0, &status);
         //if (!name)
         //{
            // It probably wasnt a valid symbol...
            // to be sure check the status code
            // "error code = 0: success";
            // "error code = -1: memory allocation failure";
            // "error code = -2: invalid mangled name";
            // "error code = -3: invalid arguments";
            //continue;
         //}
         
         // Ignore this symbols if it was generated by cint
         //if( !strncmp( name, "G__", 3) ){
         //   if (name)
            //      free(name);
            
         //   continue;
         //}

         // We needed to know if the method is a thunk(if it's just ignore it
         // dont declarte it as an error)
         //TString thunk;
         //thunk = sig->SubString("thunk");
         //if (strstr(name, "thunk")) {
         //   if (name)
         //      free(name);
            
         //   continue;
         //}

         TSymbol *symbol = new TSymbol(mangle_name, (void*)((long) sym_entry.st_value + (long) lib_base_addr));
         //symbol->fDeMangled = new TString(name);
         char* demangled = symbol->Demangle();
         symbol->SetHash(demangled);
      
         // If this symbol has no shape... dont put it inside the list
         if (symbol->Hash()) {
            //symbol_list->Add((TObject*)symbol);
            symbol_list->push_back(symbol);
            
            if(gDebug>2) {
               cerr << " --- Keeping symbol: " << demangled << endl;
               cerr << " --- hash          : " << symbol->Hash() << endl;
            }
            
            // LF: 18-06-07
            // delete the demangled name (it will be demangled again later)
            //delete symbol->fDeMangled;
            //symbol->fDeMangled = 0;
         }
         else
            delete symbol;
      
         // Dont forget to delete "name"
         // rem that __cxa_demangle will allocate the space needed for this
         if (demangled)
            free(demangled);
      }

   }
   // End of Symbol List
   //temp[symbols_chain_size] = '\0';

   // Return the symbol list and the symbol list's size
   // *symbols = temp;
   // *address = tempAddress;

   return symbol_list;
}
*/

//______________________________________________________________________________
static std::list<TSymbol*>*
SymbolList(const char* lib/*, int& ndict, int& nmem, int& nnonmem, int& ntot*/)
{
   // LF 05-07-07
   // This is the second attempt so instead of getting the symbols from the 
   // library (in memory) do it from a file called (libCore.so -> libCore.nm),
   // where such path is passed as a parameter here.
   // That file only constain a list of symbols (mangled)

   // I have to use a new HashList here since the hash changes...
   // before it was zero and now it's teh real hash... it has
   // to be modified for something more practical!!!
   std::list<TSymbol*> *symbol_list = new std::list<TSymbol*>;

   string line;
   ifstream myfile (lib);
   if (myfile.is_open()) {
      while (!myfile.eof()) {
         getline (myfile,line);
         
         TSymbol *symbol = new TSymbol(line, (void*)0);
         char* demangled = symbol->Demangle();
         symbol->SetHash(demangled);
      
         // If this symbol has no shape... dont put it inside the list
         if (symbol->Hash()) {
            symbol_list->push_back(symbol);
            
            if(gDebug>2) {
               cerr << " --- Keeping symbol: " << demangled << endl;
               cerr << " --- hash          : " << symbol->Hash() << endl;
            }
         }
         else
            delete symbol;
      
         // Dont forget to delete "name"
         // rem that __cxa_demangle will allocate the space needed for this
         if (demangled)
            free(demangled);
      }
      myfile.close();
   }

   else 
      cout << "Error: couldnt open file: " << lib << endl;
   
   return symbol_list;
}


//______________________________________________________________________________
int RegisterPointer(const char *classname, const char *method, const char *proto,
                             void *ptr, const char *mangled_name, int isconst)
{
   // Try to register the function pointer 'ptr' for a given method
   // of a given class with a certain prototype, i.e. "char*,int,float".
   // If the class is 0 the global function list will be searched (not yet).
   // Returns -1 if there was a problem and 0 if it was succesful

   struct G__ifunc_table_internal *ifunc;
   char *funcname;
   char *param;
   long index;
   long offset;
   G__ifunc_table* iref = 0;
   
   int tagnum = -1;
   ifunc = 0;

   if(strcmp(classname, "")!=0) {
      // LF 05-11-07
      // Be careful!!!!
      // G__defined_tagname can change the value of classname... why??? 
      // so something like:
      // "ROOT::Math::SMatrix<double,2,2,ROOT::Math::MatRepStd<double,2,2>>"
      // can be translated to something like 
      // "ROOT::Math::SMatrix<double,2,2,ROOT::Math::MatRepStd<double,2,2> >"
      // Which might look the same but can give unexpected results (imagine 
      // you truncate the '>' in the second case... )
      // this is very ugly...
      tagnum = G__defined_tagname(classname, 2);
      ifunc = G__struct.memfunc[tagnum];
   }

   // This means it doesn't belog to a class
   // i.e. free standing
   if (!classname || strcmp(classname, "")==0) {
      //m = Cint::G__CallFunc::GetFunc(method,proto,&offset, Cint::G__CallFunc::ExactMatch);

      /* Search for method */
      funcname = (char*)method;
      param = (char*)proto;
      iref = G__get_methodhandle3(funcname,param,G__p_ifunc,&index,&offset,0,0,2,isconst?G__CONSTFUNC:0);
   }
   else {
      //m = cinfo.GetMethod(method, proto, &offset, Cint::G__ClassInfo::ExactMatch,  Cint::G__ClassInfo::InThisScope);

      /* Search for method */
      funcname = (char*)method;
      param = (char*)proto;
      iref = G__get_methodhandle3(funcname,param,ifunc,&index,&offset,0,0,2,isconst?G__CONSTFUNC:0);
   }
   if (!iref)
      return -1;

   ifunc = G__get_ifunc_internal(iref);
   // set pointer to interface method and arguments
   //ifunc->funcptr[index] = ptr;
   // Don't do it if the name has already been written
   if(!ifunc->mangled_name[index])
      G__savestring(&ifunc->mangled_name[index],(char*)mangled_name);

   return 0;
}

//______________________________________________________________________________
void G__register_class(const char *libname, const char *clstr)
{
   // It tries to register the address of all the
   // symbols in a library.
   //
   // For this it will:
   //
   // * Load the library
   // * Traverse it and read the symbols with their addresses
   // * Demangle the symbols
   // * Try to register each pair demangled symbol<->address
   //
   // Note: we dont really use the complete demangled string
   // only its signature
   //
   // If clstr (a string with a class) is specified then we will
   // try to register only the methods belonging to that class,
   // if it's Null we will try to register all the free standing
   // functions of that library
   
   string classname;
   if(clstr)
      classname = clstr;

   
   // Take out spaces from template declarations
   string::size_type p_smaller  = classname.find_first_of("<", 0);
   string::size_type p_greater  = classname.find_last_of(">", classname.size()-1);

   if( (p_smaller != string::npos) &&
       (p_greater != string::npos)){

      // ** Pre-process a token
      string::size_type index=0;
      while(index < classname.size()){
         if( index>p_smaller && index<p_greater ) {
            if(classname[index]==' ' &&
               (index>0 && !isalpha(classname[index-1])) && 
               (index<classname.size()-1 && !isalpha(classname[index+1]))){
               classname.erase(index,1);
               p_greater  = classname.find_last_of(">", classname.size()-1);
            }
            // How is the isinteger(char) when using the stl?
            else if( (classname[index]=='0' || classname[index]=='1' || classname[index]=='2' || classname[index]=='3' || classname[index]=='4' ||
                 classname[index]=='5' || classname[index]=='6' || classname[index]=='7' || classname[index]=='8' || classname[index]=='9')
                && classname[index+1]=='u'){
               classname.erase(index+1,1);
               p_greater  = classname.find_last_of(">", classname.size()-1);
            }
            else
               index++;
         }
         else
            index++;
      }
      // ** Pre-process a token
   }




   //int ncolon=0;
   //string::size_type ind=classname.length();
   //string::size_type cstart = 0;
   //string::size_type cidx = 0;
   //while((cidx != string::npos) && (cidx < ind)){
   //   cidx = classname.find("::", cstart);
   //   
   //   if((cidx != string::npos) && (cidx < ind)){
   //      ++ncolon;
   //      cstart = cidx+2;
   //   }
   //}
   
   //if(ncolon>1)
   //   cout << "G__register_class found name space: " << classname << endl;
   
   // LF 15-10-07
   // Be careful with namespaces...
   // things like TBits::TReference::~TReference()
   // will confuse our naive algorithm... instead of just looking
   // for '::', look for the last pair of '::'
   
   //if(ncolon>0){
   //   string::size_type icolon = string::npos;
   //   string::size_type start = 0;
   //   string::size_type istart = 0;
   //   for (int i=0;i<ncolon;i++){
   //      if(icolon != string::npos)
   //         start = istart;
   //      
   //      icolon = classname.find("::", istart);
   //      istart = icolon+2;
   //   }
   //   classname = classname.substr(0, ind - (icolon+2));
   //}
   
   unsigned int  classhash = hash(classname.c_str(), classname.size());
   int nreg = 0;
   std::list<TSymbol*> *demangled = 0;
   int nerrors = 0;
   int isFreeFunc = classname.empty();
   int destInCharge = 0;    // Was this desc registered?
   int destInChargeDel = 0; // Was this desc registered?

   if(!libname || strcmp(libname,"")==0 ){
      if (gDebug > 0) {
         cerr << "****************************************" << endl;
         cerr << "Warning: Null library for class " << clstr << endl;
         cerr << "****************************************" << endl;
      }
      return;
   }
    
   // When we execute something from a macro, Register can be called with a libname
   // something.C so let's check and dont do anything if this not a library
   int len = strlen(libname);
   if (strlen(libname) > 2 && !strcmp(libname+len-2, ".h"))
      return;

   if (strlen(libname) > 3 && !(!strcmp(libname+len-3, ".sl") ||
                    !strcmp(libname+len-3, ".dl") ||
                    !strcmp(libname+len-4, ".dll")||
                    !strcmp(libname+len-4, ".DLL")||
                    !strcmp(libname+len-3, ".so") ||
                    !strcmp(libname+len-3, ".nm") ||
                    !strcmp(libname+len-2, ".a"))) {
      if (gDebug > 0) {
         cerr << "****************************************" << endl;
         cerr << "Error: " << libname << " doesnt look like a valid library" << endl;
         cerr << "****************************************" << endl;
      }
      return;
   }


   //const char *libpath = gSystem->ExpandPathName(libname);
   if (gDebug > 0) {
      cerr << "****************************************" << endl;
      cerr << "Reading symbols from library:" << libname << endl;
   }


   /*********************************/
   // small hack to avoid funny types
   char *basec, *bname;
   basec = strdup(libname);
   bname = basename(basec);

   if(strcmp(bname, "libGX11.so")==0) {
      MapDependantTypesX();
   }
   else if(strcmp(bname, "libTree.so")==0) {
      MapDependantTypesTree();
   }
   free(basec);
   /*********************************/
   
   
   // LF: 09/05/07
   // Here we have the first try to implement the symbol cache.
   // The first thing to do is to look for this library in fSymbolTable
   // Note: Remember to keep all the libraries with their full path
   //TSymbolLookup *symt = (TSymbolLookup *) (((TCint*)gInterpreter)->GetSymbolTable())->find(libname);
   multimap<string,TSymbolLookup*>           *symbolt = G__get_cache();
   multimap<string,TSymbolLookup*>::iterator  iter    = symbolt->find(string(libname));
   TSymbolLookup *symt = 0;
   if (iter != symbolt->end() ) {
         symt = (TSymbolLookup *) ((*iter).second);
   }

   //TSymbolLookup *symt = (TSymbolLookup *) ;
   if (gDebug > 0)
      cerr << "+++ Looking for the symbol table of: " << libname << endl;

   // We found it... now check is this class was already registered
   if(symt){
      if (gDebug > 0)
         cerr << "+++ The library has already been read " << endl << endl;

      if (gDebug > 0)
         cerr << "+++ Checking if the class '" << classname << "' has been registered" << endl;
      // Rem that clstr can be NULL when registering globals.
      // So let's convert that NULL to ""
      if(symt->IsRegistered(classname)){
         if (gDebug > 0)
            cerr << "+++ class '" << classname << "' has been registered" << endl << endl;

          return; // Yahoo... we dont have to do anything
      }
      else{
         // This means the library is there with all the symbols but we still
         // have to do the registering
         if (gDebug > 0)
            cerr << "+++ class '" << classname << "' has NOT been registered" << endl << endl;
         demangled = symt->fSymbols;

         // Assume this class has been registered (is what we do next
         if (gDebug > 0)
            cerr << "+++ registering symbols for class '" << classname << "'" << endl << endl;
         symt->AddRegistered(classname);
      }
      
   }
   else{
      if (gDebug > 0)
         cerr << "+++ This is a new library " << endl << endl;

      demangled = SymbolList(libname);

      //demangled = DeMangle(mangled);
      if (!demangled) {
         cerr << "Error reading/demangling the symbols" << endl;

         //delete mangled;
         return;
      }

      // Here we dont need the list with the mangled symbols anymore
      // so we can easily delete it
      //delete mangled;

      if (gDebug > 0)
         cerr << "+++ Add library '" << libname << "' to the list of libraries" << endl << endl;

      // And now that we have the symbols here we add them to the cache (for future lookups)
      symt = new TSymbolLookup(libname, demangled);
      G__get_cache()->insert(make_pair(string(libname), symt));

      // Assume this class has been registered (is what we do next)
      if (gDebug > 0)
         cerr << "+++ registering symbols for class '" << classname << "'" << endl << endl;
      symt->AddRegistered(classname);
   }

   // To register a new address we need:
   //
   // -- The class, asgiven by an object: TClass *cl
   // -- The mthod.. as a string        : const char *method
   // -- the proto.. as another string  : const char *proto
   // -- the address of the pointer     : void *ptr)
   // Now we have have the Hash table with the demangled symbols
   // and the addresses
   //
   // And that's for each entry in the hashing table

   if (gDebug > 0) {
      cerr << "****************************************" << endl;
      cerr << "Registering symbols " << endl;
   }

   //TIter next(demangled->MakeIterator());
   //while (TObject *obj = next()) {
   std::list<TSymbol*>::iterator list_iter = demangled->begin();
   while( list_iter != demangled->end()) {
      TSymbol *symbol = (TSymbol*) *list_iter;

      if( !isFreeFunc && (classhash != symbol->fClassHash) ){
         ++list_iter;
         continue;
      }

      char* deman = symbol->Demangle();
      string sig(deman);
      
      if(deman){
         free(deman);
         deman = 0;
      }
      string classstr = "";
      string classstr_noname = "";
      string protostr = "";

      // 16/04/2007
      // We cant tokenize with () only because that spoils things
      // when we have the "operator()" like in:
      //  TMatrixT<double>::operator()(int, int)
      // This is extreamly annoying and proves that this whole
      // parsing section should be rewritten following a set of rules
      // instead of the piñata paradigm

      int ncolon=0;
      string::size_type start = 0;
      string::size_type cstart = start;
      string::size_type cidx = start;
      string::size_type ind=sig.find("(");

      if(sig.find("operator()")!=string::npos)
         ind=sig.find("(", ind+1);

      // Another ugly hack... rewrite this to handle things like:
      // (anonymous namespace)::CreateIntRefConverter(long)
      // or
      // PyROOT::(anonymous namespace)::PriorityCmp(PyROOT::PyCallable*, PyROOT::PyCallable*)
      if((ind==0) || sig[ind-1]==':')
         ind=sig.find("(", ind+1);


      while((cidx != string::npos) && (cidx < ind)){
         cidx = sig.find("::", cstart);
         
         if((cidx != string::npos) && (cidx < ind)){
            ++ncolon;
            cstart = cidx+2;
         }
      }
      
      // LF 15-10-07
      // Be careful with namespaces...
      // things like TBits::TReference::~TReference()
      // will confuse our naive algorithm... instead of just looking
      // for '::', look for the last pair of '::'
      string::size_type icolon = string::npos;
      string::size_type istart = start;
      for (int i=0;i<ncolon;i++){
         if(icolon != string::npos)
            start = istart;
         
         icolon = sig.find("::", istart);
         istart = icolon+2;
      }
      //string::size_type icolon = sig.find("::");
      
      if (!isFreeFunc && (icolon != string::npos)) {
         // Dont split it with tokenize because the parameters can have things
         // like std::annoying
         classstr = sig.substr(0, icolon);
         classstr_noname = sig.substr(start, icolon-start);
         protostr = sig.substr(icolon+2, sig.size() - (icolon+2));
         //classstr = string(sig, 0, icolon);
         //protostr = sig.substr(icolon+2, sig.size() - (icolon+2));

         if (gDebug > 0) {
            cerr << "classstr : " << classstr << endl;
            cerr << "protostr : " << protostr << endl;
         }
      }
      else if (!isFreeFunc && (icolon == string::npos)) {
         // We get something without colons when we try to register
         // a class... ignore it
         ++list_iter;
         continue;
      }
      else if( isFreeFunc && (icolon != string::npos)) {
         // What want to register free funcs but this one has ::
         // so we just ignore it
         ++list_iter;
         continue;
      }
      else if( isFreeFunc && (icolon == string::npos)) {
         // This means it's a free function and we have to register ir
         protostr = string(sig);
      }

      if (!isFreeFunc && (classstr.empty() || protostr.empty()) ) {
         if (gDebug > 0)
            cerr << "Couldnt find class or method for symbol : " << sig << endl << endl;
         nerrors++;
         ++list_iter;
         continue;
      }

      // How can we get rid of this class in a clean way?
      // The problem is that this class is declared in TTree.h but there is no
      // .h for itself and there is no dictionary so CInt does not know about it
      // but we will find it when we read the symbols :/ ....
      // maybe we should change things like TTree::TFriendLock::TFriendLock to
      // just TFriendLock::TFriendLock but I'm not sure about the consequences
      // that can have.
      if(clstr && strcmp(clstr, "TTree")==0 && protostr.find("TFriendLock::")!=string::npos ) {
         ++list_iter;
         continue;
      }
      
      // LF: 09-05-07
      // for the moment ignore non-members overloaded operators... we will deal with them
      // later
      //if(!clstr && protostr.find("operator")!=string::npos){
      //   ++list_iter;
      //   continue;
      //}

      // LF: 10/05/07
      // this is small hack (yes... again). Let's ignore all the functions
      // that belong to std and start with __ (why? they are weird)
      if (symbol->fClassHash==hash("std", 3) && protostr.at(0)=='_'){
         ++list_iter;
         continue;
      }

      // Here we have to parse it again to detect
      // what we can find between the parenthesis
      string signature = "";
      string::size_type open  = protostr.find('(');
      string::size_type close = protostr.rfind(')');

      if(protostr.find("operator()")!=string::npos)
         open=protostr.find("(", open+1);

      // The name of the method is the proto until the first (
      string methodstr(protostr, 0, open);

      if (methodstr=="operator" && protostr.at(open)=='(' && protostr.at(open+1)==')'){
         if (gDebug > 0)
            cerr << " This thing is tricky (operator). be aware" << endl;
         methodstr += "()";
         open += 2;
      }
      if (gDebug > 0)
         cerr << "methodstr: " << methodstr << endl;

      // Very annoying class... check it later
      if(symbol->fIsConst && methodstr=="TFormulaPrimitive"){
         ++list_iter;
         continue;
      }

      // Ignore all cons except for the on in charge
      //if (symbol->fIsConst && !symbol->fIsConstInC){
      //   ++list_iter;
      //   continue;
      //}

      // Ignore all des except for the on in charge
      //if (symbol->fIsDestInC && !symbol->fIsDestInC){
      //   ++list_iter;
      //   continue;
      //}

      if ( (close - open) > 1)
         signature += protostr.substr(open+1, (close-open)-1 );

      // Here we have a problem....
      // a normal prototype looks like:
      // TH1::Add(TH1 const*, TH1 const*, double, double)
      //
      // but Cint doesnt like this kind of constness
      // the only thing it wants to see is:
      // TH1::Add(const TH1 *, const TH1 *, double, double)
      //
      // Ideally, I should modify Cint to accept both
      // but since I'm only trying to create a proof of concept
      // I will hack my way through for the moment and consider
      // the Cint changes later on.

      // (so... let's change the format for comething more suitable)
      string newsignature = "";
      if (!signature.empty()) {
         string delim(",");
         
         // go to the first pos
         string::size_type lpos = signature.find_first_not_of(delim, 0);
         // first delim
         string::size_type pos  = signature.find_first_of(delim, lpos);

         if(pos == string::npos && lpos != pos)
            pos = signature.size();
         else{
            // LF 05-11-07
            // We can have something like:
            // Were one of the parameters includes a comma. I would think it's only possible
            // when we have templates and an option is to ignore a comma if it's between "<>"...
            // ROOT::Math::SVector<double, 2u>::operator=(ROOT::Math::SVector<double, 2u> const&)
            string::size_type pos_sm  = signature.find_first_of("<", lpos);
            if(pos_sm != string::npos && pos_sm < pos){
               string::size_type pos_gr  = signature.find_first_of(">", pos_sm);
               if(pos_gr != string::npos && pos < pos_gr && pos > pos_sm)
                  pos =  signature.find_first_of(delim, pos_gr); 
            }
            
            if(pos == string::npos && lpos != pos)
               pos = signature.size();
         }

         int k = 0;
         while (pos != string::npos || lpos != string::npos) {
            // new token
            string paramtoken = signature.substr(lpos, pos - lpos);
            string newparam;
		
            // LF 05-11-07
            // We can get something like:
            // ROOT::Math::SVector<double, 2u>
            // When we have actually declared
            // ROOT::Math::SVector<double,2>
            // in ROOT... so the hashes wont match..
            // try to convert the former in the latter.
            
            string::size_type pos_s  = paramtoken.find_first_of("<", 0);
            string::size_type pos_g  = paramtoken.find_last_of(">", paramtoken.size()-1);

            if( (pos_s != string::npos) &&
                (pos_g != string::npos)){

               // ** Pre-process a token
               string::size_type index=0;
               while(index < paramtoken.size()){
                  if( index>pos_s && index<pos_g ) {
                     if(paramtoken[index]==' ' &&
                        (index>0 && !isalpha(paramtoken[index-1])) && 
                        (index<paramtoken.size()-1 && !isalpha(paramtoken[index+1]))){
                        paramtoken.erase(index,1);
                        pos_g  = paramtoken.find_last_of(">", paramtoken.size()-1);
                     }
                     // How is the isinteger(char) when using the stl?
                     else if( (paramtoken[index]=='0' || paramtoken[index]=='1' || paramtoken[index]=='2' || paramtoken[index]=='3' || paramtoken[index]=='4' ||
                          paramtoken[index]=='5' || paramtoken[index]=='6' || paramtoken[index]=='7' || paramtoken[index]=='8' || paramtoken[index]=='9')
                         && paramtoken[index+1]=='u'){
                        pos_g  = paramtoken.find_last_of(">", paramtoken.size()-1);
                        paramtoken.erase(index+1,1);                  
                     }
                     else
                        index++;
                  }
                  else
                     index++;
               }
               // ** Pre-process a token
            }

           if (!paramtoken.empty() && paramtoken.find("const")!=string::npos ) {
              string delim_param(" ");
              string singleparamold;

              // go to the first pos
              string::size_type lpos_param = paramtoken.find_first_not_of(delim_param, 0);
              // first delim
              string::size_type pos_param  = paramtoken.find_first_of(delim_param, lpos_param);
              if(pos_param == string::npos && lpos_param != pos_param)
                 pos_param = paramtoken.size();

              // Paramtoken could be something like "TParameter<long long>"
              // in that case the tokenizing by space is wrong!!!
              string::size_type pos_smaller  = paramtoken.find_first_of("<", lpos_param);
              
              // If we find a "<" between the start and the space,
              // then we look for a ">" after the space
              if(pos_smaller != string::npos && pos_smaller < pos_param){
                 string::size_type pos_greater  = paramtoken.find_last_of(">", paramtoken.size()-1);
                 if(pos_greater != string::npos /*&& pos_greater > pos_param*/)
                    pos_param = pos_greater+1;
                 else
                    cerr << "Error parsing method: " << methodstr << " signature:" << signature << endl;
              }

              int l = 0;
              while (pos_param != string::npos || lpos_param != string::npos) {
                 // new token
                 string singleparam = paramtoken.substr(lpos_param, pos_param - lpos_param);
                 if (l > 0 && singleparam == "const*"){
                    newparam += "const ";
                    newparam += singleparamold;//paramtoken.substr(lpos_param, pos_param - lpos_param);
                    newparam += " *";
                 }
                 else if (l > 0 && singleparam == "const&"){
                    newparam += "const ";
                    newparam += singleparamold;//paramtoken.substr(lpos_param, pos_param - lpos_param);
                    newparam += " &";
                 }
                 else if (l > 0 && singleparam == "const*&"){
                    newparam += "const ";
                    newparam += singleparamold;//paramtoken.substr(lpos_param, pos_param - lpos_param);
                    newparam += " *&";
                 }
                 else if (l > 0 && singleparam == "const**"){
                    newparam += "const ";
                    newparam += singleparamold;//paramtoken.substr(lpos_param, pos_param - lpos_param);                   
                    newparam += " **";
                 }
                 // skip delim
                 lpos_param = paramtoken.find_first_not_of(delim_param, pos_param);
                 // find next token
                 pos_param  = paramtoken.find_first_of(delim_param, lpos_param);
                 if(pos_param == string::npos && lpos_param != pos_param)
                    pos_param = paramtoken.size();
 
                 singleparamold = singleparam;
                 ++l;
             }
           }
           else if(!paramtoken.empty() && paramtoken.find("const")==string::npos)
              newparam = paramtoken;

           if(k > 0)
               newsignature += ",";

           if(newparam.empty())
              newsignature += paramtoken;
           else
              newsignature += newparam;
           
           // skip delim
           lpos = signature.find_first_not_of(delim, pos);
           // find next token
           pos  = signature.find_first_of(delim, lpos);
         
           if(pos == string::npos && lpos != pos)
              pos = signature.size();
           ++k;
         }
      }
      if (gDebug > 0) {
         cerr << "--- OLD Signature: " << signature << endl;
         cerr << "--- NEW Signature: " << newsignature << endl;
      }

      // LF: 21/05/7
      // We have another problem, certain symbols have names like: 
      // shared_ptr<Base>* const shared_ptr<Base>::n3<Base>(shared_ptr<Base> const&)
      // which obviously doesnt give us a good name for a class. And as far as I can tell,
      // only represents the return type... so lets strip it out to obtain only
      // the name of the class

      string::size_type ispace  = classstr.rfind(" ", 0);

      string finalclass;
      if(ispace != string::npos){
         // Let's just assume this is the last entry (can a classname contain spaces? )
         finalclass = signature.substr(ispace, classstr.size());
      }
      else
         finalclass = classstr;


      // LF 12-10-07
      // CInt doesn't believe that a constructor can be different from the name of
      // the class. So when we have things like:
      //
      // TParameter<double>::TParameter() 
      //
      // CInt will just think it's 
      // 
      // TParameter<double>::TParameter<double>()
      // 
      // Changint his in CInt would probably requiere more changes than changing the
      // real name to what Cint expects.
      
      // 11-10-07
      // Get rid of the "<>" part in something like TParameter<float>::TParameter()
      if(symbol->fIsConst){
         string::size_type itri = finalclass.find("<");
      
         if(itri != string::npos){
            methodstr = finalclass;
         }
      }
      else if(symbol->fIsDest){
         string::size_type itri = finalclass.find("<");
      
         if(itri != string::npos){
            methodstr = "~" + finalclass;
         }
      }

      // LF 31-07-07
      // I forgot something else....
      // the stupid constness (rem two functions can vary only by their constness)
      // so lets check it here now
      int isconst=0;

      string::size_type lpar = sig.find_last_of(")");
      string::size_type pos_par  = sig.find_first_of("const", lpar);
      if(pos_par != string::npos)
         isconst=1;

      // LF 18-10-07
      // How can we deal with things like:
      // 
      // TClass::GetClass(char const*, bool)::full_string_name
      // TClass::GetClass(char const*, bool)::__PRETTY_FUNCTION__
      if(sig.find_first_of("::", lpar)!=string::npos){
         ++list_iter;
         continue;
      }

      // LF 16-15-07
      // Another ugly hack
      // deal with annoying things that don't even look like a function
      /*
        void (*std::for_each<__gnu_cxx::__normal_iterator<PyROOT::(anonymous namespace)::PyError_t*, std::vector<PyROOT::(anonymous namespace)::PyError_t, std::allocator<PyROOT::(anonymous namespace)::PyError_t> > >, void (*)(PyROOT::(anonymous namespace)::PyError_t&)>(__gnu_cxx::__normal_iterator<PyROOT::(anonymous namespace)::PyError_t*, std::vector<PyROOT::(anonymous namespace)::PyError_t, std::allocator<PyROOT::(anonymous namespace)::PyError_t> > >, __gnu_cxx::__normal_iterator<PyROOT::(anonymous namespace)::PyError_t*, std::vector<PyROOT::(anonymous namespace)::PyError_t, std::allocator<PyROOT::(anonymous namespace)::PyError_t> > >, void (*)(PyROOT::(anonymous namespace)::PyError_t&)))(PyROOT::(anonymous namespace)::PyError_t&)
      */
      if(sig.find("PyError_t")!=string::npos) {
         ++list_iter;
         continue;
      }
      
      //if(!symbol->fFunc){
      //   if (gDebug > 0)
      //      cerr << "xxx The address is 0. the method wont be registered" << endl << endl;
      //}
      char *tmpstr = new char[classname.size()+56]; // it could be changed by RegisterPointer and the size can be bigger
      strcpy(tmpstr, classname.c_str());
      if( RegisterPointer(tmpstr, /*finalclass.c_str(),*/ methodstr.c_str(), 
                          newsignature.c_str(), symbol->fFunc, symbol->fMangled.c_str(),
                          isconst) == -1){
         // yahoo.... we can finally call our register method
         if (gDebug > 0) {
            cerr << "xxx Couldnt register the method: " << methodstr << endl;
            cerr << "xxx from the class    : " << finalclass << endl;
            cerr << "xxx classname    : " << classname << endl;
            cerr << "xxx with the signature: " << newsignature << endl;
            cerr << "xxx with the address : " << symbol->fFunc << endl << endl ;
         }
         nerrors++;
      }
      else {
         if (gDebug > 0) {
            cerr << " *** Method registered  : " << methodstr << endl;
            cerr << " *** from the class     : " << finalclass << endl;
            cerr << " *** with the signature : " << newsignature << endl;
            cerr << " *** with the address : " << symbol->fFunc << endl << endl ;
         }
         nreg++;
      }
      delete tmpstr;

      //std::list<TSymbol*>::iterator old_iter = list_iter;
      ++list_iter;
      demangled->remove(symbol);
      delete symbol;
   }
   if (gDebug > 0)
      cerr << "****************************************" << endl << endl;


   // We are done with the list of demangled symbols....
   // now is time to clean up all that space
   //if(demangled)
   //   delete demangled;
   //if(algorithm == 1 || algorithm == 2){
   //   std::for_each( demangled->begin(), demangled->end(), ptr_delete() );
   //   demangled->clear();
      //delete demangled;
   //}

   if (gDebug > 0) {
      cerr << "*************************************" << endl;
      cerr << " Number of symbols registered : " << nreg << endl;
      cerr << " Number of errors             : " << nerrors << endl;
      cerr << "*************************************" << endl << endl;
   }
}
