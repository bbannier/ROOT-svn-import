// @(#)root/meta:$Id$
// Author: Fons Rademakers   01/03/96

/*************************************************************************
 * Copyright (C) 1995-2000, Rene Brun and Fons Rademakers.               *
 * All rights reserved.                                                  *
 *                                                                       *
 * For the licensing terms see $ROOTSYS/LICENSE.                         *
 * For the list of contributors see $ROOTSYS/README/CREDITS.             *
 *************************************************************************/

//////////////////////////////////////////////////////////////////////////
//                                                                      //
// This class defines an interface to the CINT C/C++ interpreter made   //
// by Masaharu Goto from HP Japan.                                      //
//                                                                      //
// CINT is an almost full ANSI compliant C/C++ interpreter.             //
//                                                                      //
//////////////////////////////////////////////////////////////////////////

#include "TCint.h"
#include "G__ci.h"
#include "TROOT.h"
#include "TApplication.h"
#include "TGlobal.h"
#include "TDataType.h"
#include "TClass.h"
#include "TClassEdit.h"
#include "TBaseClass.h"
#include "TDataMember.h"
#include "TMethod.h"
#include "TMethodArg.h"
#include "TObjArray.h"
#include "TObjString.h"
#include "TString.h"
#include "THashList.h"
#include "TOrdCollection.h"
#include "TVirtualPad.h"
#include "TSystem.h"
#include "TVirtualMutex.h"
#include "TError.h"
#include "TEnv.h"
#include "THashTable.h"
#include "CallFunc.h"


#include <cxxabi.h>
#include <dlfcn.h>
#include <link.h>
#include <iostream>

#include <vector>
#include <list>
#include <set>
#include <string>
using namespace std;

static ULong_t hash(const void *txt, Int_t ntxt)
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

   static const ULong_t utab[] = {
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

   static const ULong_t msk[] = { 0x11111111, 0x33333333, 0x77777777, 0xffffffff };

   const UChar_t *uc = (const UChar_t *) txt;
   ULong_t uu = 0;
   union {
      ULong_t  u;
      UShort_t s[2];
   } u;
   u.u = 0;
   Int_t i, idx;

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
   //UInt_t    fHash;
   UInt_t    fClassHash;

   // This points directly to the name in the library
   const char* fMangled;
      
   Bool_t    fIsDest;
   Bool_t    fIsDestInC;
   Bool_t    fIsDestInCDel;
   Bool_t    fIsConst;
   Bool_t    fIsConstInC;

   TSymbol()
      : fFunc(0), /*fHash(0),*/ fClassHash(0), fMangled(0),
     fIsDest(kFALSE), fIsDestInC(kFALSE), fIsDestInCDel(kFALSE), fIsConst(kFALSE), fIsConstInC(kFALSE)
   {
   }

   TSymbol(const char* mangled, void* func)
   : fFunc(func), /*fHash(0),*/ fClassHash(0), fMangled(mangled), fIsDest(kFALSE), 
     fIsDestInC(kFALSE), fIsDestInCDel(kFALSE), fIsConst(kFALSE), fIsConstInC(kFALSE)
   {
      fMangled = mangled; 
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

         string::size_type icolon = demangled.find("::", start);
         if (icolon != string::npos && icolon < ind){
            // Now hash the class name also
            // (it will make things easier when we try to register
            // symbols by class)
            string classname = string(demangled, start, icolon);
            string protoname = demangled.substr(icolon+2, ind - (icolon+2));

            // ** constructors
            if ( classname == protoname)
            {
               fIsConst = kTRUE;

               // if this not the constructor in charge then just continue
               string mang(classname);
               mang += "C1";
               
               //TString sub = fMangled->SubString(mang);
               if(!strstr(fMangled, mang.c_str())) {
                  //if (sub.IsNull()) {
                  // This is not the constructor in-charge... ignore it
                  return;
               }
               fIsConstInC = kTRUE;
            }

            // ** destructors
            string dest("~");
            dest += classname;
            if ( dest == protoname)
            {
               fIsDest = kTRUE;
               // if this not the constructor in charge then just continue
               string mang0(classname);
               string mang1(classname);
               mang0 += "D0";
               mang1 += "D1";
               
               //if (!(fMangled->SubString(mang0)).IsNull()) {
               if(strstr(fMangled, mang0.c_str())) {
               // This is the deleting constructor
                  fIsDestInCDel = kTRUE;
               }
               //else if(!(fMangled->SubString(mang1)).IsNull()){
               else if( strstr(fMangled, mang1.c_str()) ){
                  // This is the in-charge (non deleting) constructor
                  fIsDestInC = kTRUE;
                  
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

   ULong_t Hash() const
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

      if(!fMangled) {
         cerr << "Fatal error: Demangle() fMangled is NULL"<< endl;
         return 0;
      }

      // LF: 22/05/07
      // Instead of copying the fMangled name just do the demangling
      // I dont know why the symbols start with __Z if the ABI wants them
      // to start with _Z... until understanding it just hack your way through
      const char *cline = fMangled;
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
   TObjArray  *fRegistered;
   //TList      *fSymbols;
   std::list<TSymbol*> *fSymbols;

   TSymbolLookup()
   {
      //fLibname    = 0;
      fRegistered = 0;
      //fSymbols    = 0;
   }

   TSymbolLookup(const char *libname, std::list<TSymbol*> *symbols)
   {
      fRegistered = 0;
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
      delete fRegistered;
      //delete fSymbols;
      std::for_each( fSymbols->begin(), fSymbols->end(), ptr_delete() );
      fSymbols->clear();
   }
   
   void AddRegistered(string &classname)
   {
      // This will add a new classname to the list of registered classes
      if(!fRegistered) {
         fRegistered = new TObjArray();
         fRegistered->SetOwner();
      }

      fRegistered->Add((TObject*)new TObjString(classname.c_str()));
   }

   ULong_t Hash() const
   {
      // This is just the name of the library
      // (always remember that this is the fullpath)

      if (fLibname.size())
         return hash(fLibname.c_str(), fLibname.size());

      return 0;
   }

   Bool_t IsRegistered(string &classname)
   {
      // Let's check if this class has already been registered
      if(fRegistered && fRegistered->FindObject(classname.c_str()))
         return kTRUE;

      return kFALSE;
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

R__EXTERN int optind;

// reference cint includes
// make sure fproto.h is loaded (it was excluded in TCint.h)
//#undef G__FPROTO_H
//#include "fproto.h"
#include "Api.h"

extern "C" int ScriptCompiler(const char *filename, const char *opt) {
   return gSystem->CompileMacro(filename, opt);
}

extern "C" int IgnoreInclude(const char *fname, const char *expandedfname) {
   return gROOT->IgnoreInclude(fname,expandedfname);
}

extern "C" void TCint_UpdateClassInfo(char *c, Long_t l) {
   TCint::UpdateClassInfo(c, l);
}

extern "C" int TCint_AutoLoadCallback(char *c, char *l) {
   ULong_t varp = G__getgvp();
   G__setgvp(G__PVOID);
   string cls(c);
   int result =  TCint::AutoLoadCallback(cls.c_str(), l);
   G__setgvp(varp);
   return result;
}

extern "C" void *TCint_FindSpecialObject(char *c, G__ClassInfo *ci, void **p1, void **p2) {
   return TCint::FindSpecialObject(c, ci, p1, p2);
}

// It is a "fantom" method to synchronize user keyboard input
// and ROOT prompt line (for WIN32)
const char *fantomline = "TRint::EndOfLineAction();";

void* TCint::fgSetOfSpecials = 0;

ClassImp(TCint)

//______________________________________________________________________________
TCint::TCint(const char *name, const char *title) : TInterpreter(name, title)
{
   // Initialize the CINT interpreter interface.

   fMore      = 0;
   fPrompt[0] = 0;
   fMapfile   = 0;
   fRootMapFiles = 0;
   fSymbolTable = 0;
   fLockProcessLine = kTRUE;

   G__RegisterScriptCompiler(&ScriptCompiler);
   G__set_ignoreinclude(&IgnoreInclude);
   G__InitUpdateClassInfo(&TCint_UpdateClassInfo);
   G__InitGetSpecialObject(&TCint_FindSpecialObject);

   fDictPos.ptype = 0;
   fDictPosGlobals.ptype = 0;

   // Create the table Libraries<->Symbols
   fSymbolTable = new multimap<string, TSymbolLookup*>;
   //fSymbolTable->SetOwner();

   ResetAll();

#ifndef R__WIN32
   optind = 1;  // make sure getopt() works in the main program
#endif

   // Make sure that ALL macros are seen as C++.
   G__LockCpp();

   // Register the callback for registering symbols 
   if (gDebug > 0)
      Info("Load", " Registering TCint::Register in Cint :%p", (void*)TCint::Register);
   G__memfunc_register_callback(TCint::Register);
}

//______________________________________________________________________________
TCint::~TCint()
{
   // Destroy the CINT interpreter interface.

   if (fMore != -1) {
      // only close the opened files do not free memory:
      // G__scratch_all();
      G__close_inputfiles();
   }

   free(fDictPos.ptype);
   free(fDictPosGlobals.ptype);
   delete fMapfile;
   delete fRootMapFiles;
   delete fSymbolTable;
}

//______________________________________________________________________________
void TCint::ClearFileBusy()
{
   // Reset CINT internal state in case a previous action was not correctly
   // terminated by G__init_cint() and G__dlmod().

   G__clearfilebusy(0);
}

//______________________________________________________________________________
void TCint::ClearStack()
{
   // Delete existing temporary values

   G__clearstack();
}

//______________________________________________________________________________
Int_t TCint::InitializeDictionaries()
{
   // Initialize all registered dictionaries. Normally this is already done
   // by G__init_cint() and G__dlmod().

   // LF: 24/04/07
   // Fix compiler dependant types
   // This should go inside TCint::TCint but at that moment
   // gApplication is 0. Where else can we put it?
   MapDependantTypes();

   // LF: 03/05/07
   // Since libCore is loaded before loading TCint we wont be able to register
   // all the static function (so we wont be able to call them without the 
   // stubs later). So let's register them here... again, this is a temp
   // fix and a more sutable place has to be found for this call
   // (the same for libCint.so and libRint.so)
   Register("libCore.so", 0);
   Register("libCore.so", "TDirectory");
   //Register("libCint.so", 0);
   //Register("libRint.so", 0);

   return G__call_setup_funcs();
}

//______________________________________________________________________________
void TCint::EnableAutoLoading()
{
   // Enable the automatic loading of shared libraries when a class
   // is used that is stored in a not yet loaded library. Uses the
   // information stored in the class/library map (typically
   // $ROOTSYS/etc/system.rootmap).

   G__set_class_autoloading_callback(&TCint_AutoLoadCallback);
   LoadLibraryMap();
}

//______________________________________________________________________________
void TCint::EndOfLineAction()
{
   // It calls a "fantom" method to synchronize user keyboard input
   // and ROOT prompt line.

   ProcessLineSynch(fantomline);
}

//______________________________________________________________________________
Bool_t TCint::IsLoaded(const char* filename) const
{
   // Return true if the file has already been loaded by cint.

   // We will try in this order:
   //   actual filename
   //   filename as a path relative to
   //            the include path
   //            the shared library path

   G__SourceFileInfo file(filename);
   if (file.IsValid()) { return kTRUE; };

   char *next = gSystem->Which(TROOT::GetMacroPath(), filename, kReadPermission);
   if (next) {
      file.Init(next);
      delete [] next;
      if (file.IsValid()) { return kTRUE; };
   }

   TString incPath = gSystem->GetIncludePath(); // of the form -Idir1  -Idir2 -Idir3
   incPath.Append(":").Prepend(" ");
   incPath.ReplaceAll(" -I",":");       // of form :dir1 :dir2:dir3
   while ( incPath.Index(" :") != -1 ) {
      incPath.ReplaceAll(" :",":");
   }
   incPath.Prepend(".:");
   incPath.Append(":$ROOTSYS/cint/include:$ROOTSYS/cint/stl");
   next = gSystem->Which(incPath, filename, kReadPermission);
   if (next) {
      file.Init(next);
      delete [] next;
      if (file.IsValid()) { return kTRUE; };
   }

   next = gSystem->DynamicPathName(filename,kTRUE);
   if (next) {
      file.Init(next);
      delete [] next;
      if (file.IsValid()) { return kTRUE; };
   }

   return kFALSE;
}

//______________________________________________________________________________
Int_t TCint::Load(const char *filename, Bool_t system)
{
   // Load a library file in CINT's memory.
   // if 'system' is true, the library is never unloaded.

   R__LOCKGUARD2(gCINTMutex);
   int i;
   if (!system)
      i = G__loadfile(filename);
   else
      i = G__loadsystemfile(filename);

   UpdateListOfTypes();

   return i;
}

//______________________________________________________________________________
void TCint::LoadMacro(const char *filename, EErrorCode *error)
{
   // Load a macro file in CINT's memory.

   ProcessLine(Form(".L %s", filename), error);
}

//______________________________________________________________________________
Long_t TCint::ProcessLine(const char *line, EErrorCode *error)
{
   // Let CINT process a command line.
   // If the command is executed and the result of G__process_cmd is 0,
   // the return value is the int value corresponding to the result of the command
   // (float and double return values will be truncated).

   Long_t ret = 0;
   if (gApplication) {
      if (gApplication->IsCmdThread()) {
         if (gGlobalMutex && !gCINTMutex && fLockProcessLine) {
            gGlobalMutex->Lock();
            if (!gCINTMutex)
               gCINTMutex = gGlobalMutex->Factory(kTRUE);
            gGlobalMutex->UnLock();
         }
         R__LOCKGUARD(fLockProcessLine ? gCINTMutex : 0);
         gROOT->SetLineIsProcessing();

         G__value local_res;
         G__setnull(&local_res);

         // It checks whether the input line contains the "fantom" method
         // to synchronize user keyboard input and ROOT prompt line
         if (strstr(line,fantomline)) {
            G__free_tempobject();
            TCint::UpdateAllCanvases();
         } else {
            int local_error = 0;

            int prerun = G__getPrerun();
            G__setPrerun(0);
            ret = G__process_cmd((char *)line, fPrompt, &fMore, &local_error, &local_res);
            G__setPrerun(prerun);
            if (local_error == 0 && G__get_return(&fExitCode) == G__RETURN_EXIT2) {
               ResetGlobals();
               gApplication->Terminate(fExitCode);
            }
            if (error)
               *error = (EErrorCode)local_error;
         }

         if (ret == 0) ret = G__int_cast(local_res);

         gROOT->SetLineHasBeenProcessed();
      } else {
         ret = ProcessLineAsynch(line, error);
      }
   } else {
      if (gGlobalMutex && !gCINTMutex && fLockProcessLine) {
         gGlobalMutex->Lock();
         if (!gCINTMutex)
            gCINTMutex = gGlobalMutex->Factory(kTRUE);
         gGlobalMutex->UnLock();
      }
      R__LOCKGUARD(fLockProcessLine ? gCINTMutex : 0);
      gROOT->SetLineIsProcessing();

      G__value local_res;
      G__setnull(&local_res);

      int local_error = 0;

      int prerun = G__getPrerun();
      G__setPrerun(0);
      ret = G__process_cmd((char *)line, fPrompt, &fMore, &local_error, &local_res);
      G__setPrerun(prerun);
      if (local_error == 0 && G__get_return(&fExitCode) == G__RETURN_EXIT2) {
         ResetGlobals();
         exit(fExitCode);
      }
      if (error)
         *error = (EErrorCode)local_error;

      if (ret == 0) ret = G__int_cast(local_res);

      gROOT->SetLineHasBeenProcessed();
   }
   return ret;
}

//______________________________________________________________________________
Long_t TCint::ProcessLineAsynch(const char *line, EErrorCode *error)
{
   // Let CINT process a command line asynch.

   return ProcessLine(line, error);
}

//______________________________________________________________________________
Long_t TCint::ProcessLineSynch(const char *line, EErrorCode *error)
{
   // Let CINT process a command line synchronously, i.e we are waiting
   // it will be finished.

   if (gApplication) {
      if (gApplication->IsCmdThread())
         return ProcessLine(line, error);
      return 0;
   }
   return ProcessLine(line, error);
}

//______________________________________________________________________________
Long_t TCint::Calc(const char *line, EErrorCode *error)
{
   // Directly execute an executable statement (e.g. "func()", "3+5", etc.
   // however not declarations, like "Int_t x;").

   Long_t result;

#ifdef R__WIN32
   // Test on ApplicationImp not being 0 is needed because only at end of
   // TApplication ctor the IsLineProcessing flag is set to 0, so before
   // we can not use it.
   if (gApplication && gApplication->GetApplicationImp()) {
      while (gROOT->IsLineProcessing() && !gApplication) {
         Warning("Calc", "waiting for CINT thread to free");
         gSystem->Sleep(500);
      }
      gROOT->SetLineIsProcessing();
   }
#endif
   R__LOCKGUARD2(gCINTMutex);
   result = (Long_t) G__int_cast(G__calc((char *)line));
   if (error) *error = (EErrorCode)G__lasterror();

#ifdef R__WIN32
   if (gApplication && gApplication->GetApplicationImp())
      gROOT->SetLineHasBeenProcessed();
#endif

   return result;
}

//______________________________________________________________________________
void TCint::PrintIntro()
{
   // Print CINT introduction and help message.

   Printf("\nCINT/ROOT C/C++ Interpreter version %s", G__cint_version());
   Printf("Type ? for help. Commands must be C++ statements.");
   Printf("Enclose multiple statements between { }.");
}

//______________________________________________________________________________
void TCint::RecursiveRemove(TObject *obj)
{
   // Delete object from CINT symbol table so it can not be used anymore.
   // CINT object are always on the heap.

   if (obj->IsOnHeap() && fgSetOfSpecials && !((std::set<TObject*>*)fgSetOfSpecials)->empty()) {
      std::set<TObject*>::iterator iSpecial = ((std::set<TObject*>*)fgSetOfSpecials)->find(obj);
      if (iSpecial != ((std::set<TObject*>*)fgSetOfSpecials)->end()) {
         DeleteGlobal(obj);
         ((std::set<TObject*>*)fgSetOfSpecials)->erase(iSpecial);
      }
   }
}

//______________________________________________________________________________
void TCint::Reset()
{
   // Reset the CINT state to the state saved by the last call to
   // TCint::SaveContext().

   G__scratch_upto(&fDictPos);
}

//______________________________________________________________________________
void TCint::ResetAll()
{
   // Reset the CINT state to its initial state.

   G__init_cint("cint +V");
   G__init_process_cmd();
}

//______________________________________________________________________________
void TCint::ResetGlobals()
{
   // Reset the CINT global object state to the state saved by the last
   // call to TCint::SaveGlobalsContext().

   G__scratch_globals_upto(&fDictPosGlobals);
}

//______________________________________________________________________________
void TCint::RewindDictionary()
{
   // Rewind CINT dictionary to the point where it was before executing
   // the current macro. This function is typically called after SEGV or
   // ctlr-C after doing a longjmp back to the prompt.

   G__rewinddictionary();
}

//______________________________________________________________________________
Int_t TCint::DeleteGlobal(void *obj)
{
   // Delete obj from CINT symbol table so it cannot be accessed anymore.
   // Returns 1 in case of success and 0 in case object was not in table.

   return G__deleteglobal(obj);
}

//______________________________________________________________________________
void TCint::SaveContext()
{
   // Save the current CINT state.

   G__store_dictposition(&fDictPos);
}

//______________________________________________________________________________
void TCint::SaveGlobalsContext()
{
   // Save the current CINT state of global objects.

   G__store_dictposition(&fDictPosGlobals);
}

//______________________________________________________________________________
void TCint::UpdateListOfGlobals()
{
   // Update the list of pointers to global variables. This function
   // is called by TROOT::GetListOfGlobals().

   R__LOCKGUARD2(gCINTMutex);
   G__DataMemberInfo t, *a;
   while (t.Next()) {
      // if name cannot be obtained no use to put in list
      if (t.IsValid() && t.Name()) {
         // first remove if already in list
         TGlobal *g = (TGlobal *)gROOT->fGlobals->FindObject(t.Name());
         if (g) {
            gROOT->fGlobals->Remove(g);
            delete g;
         }
         a = new G__DataMemberInfo(t);
         gROOT->fGlobals->Add(new TGlobal(a));
      }
   }
}

//______________________________________________________________________________
void TCint::UpdateListOfGlobalFunctions()
{
   // Update the list of pointers to global functions. This function
   // is called by TROOT::GetListOfGlobalFunctions().

   R__LOCKGUARD2(gCINTMutex);
   G__MethodInfo t, *a;
   void* vt =0;

   while (t.Next()) {
      // if name cannot be obtained no use to put in list
      if (t.IsValid() && t.Name()) {
         Bool_t needToAdd = kTRUE;
         // first remove if already in list
         TList* listFuncs = ((THashTable*)(gROOT->fGlobalFunctions))->GetListForObject(t.Name());
         if (listFuncs && (vt = (void*)t.InterfaceMethod())) {
            Int_t prop = -1;
            TIter iFunc(listFuncs);
            TFunction* f = 0;
            Bool_t foundStart = kFALSE;
            while (needToAdd && (f = (TFunction*)iFunc())) {
               if (strcmp(f->GetName(),t.Name())) {
                  if (foundStart) break;
                  continue;
               }
               foundStart = kTRUE;
               if (vt == f->InterfaceMethod()) {
                  if (prop == -1)
                     prop = t.Property();
                  needToAdd = !((prop & G__BIT_ISCOMPILED)
                                || t.GetMangledName() == f->GetMangledName());
               }
            }
         }

         if (needToAdd) {
            a = new G__MethodInfo(t);
            gROOT->fGlobalFunctions->Add(new TFunction(a));
         }
      }
   }
}

//______________________________________________________________________________
void TCint::UpdateListOfTypes()
{
   // Update the list of pointers to Datatype (typedef) definitions. This
   // function is called by TROOT::GetListOfTypes().

   R__LOCKGUARD2(gCINTMutex);

   // Remember the index of the last type that we looked at,
   // so that we don't keep reprocessing the same types.
   static int last_typenum = -1;

   // Also remember the count from the last time the dictionary
   // was rewound.  If it's been rewound since the last time we've
   // been called, then we recan everything.
   static int last_scratch_count = 0;
   int this_scratch_count = G__scratch_upto(0);
   if (this_scratch_count != last_scratch_count) {
      last_scratch_count = this_scratch_count;
      last_typenum = -1;
   }

   // Scan from where we left off last time.
   G__TypedefInfo t (last_typenum);
   while (t.Next()) {
      const char* name = t.Name();
      if (gROOT && gROOT->fTypes && t.IsValid() && name) {
         TDataType *d = (TDataType *)gROOT->fTypes->FindObject(name);
         // only add new types, don't delete old ones with the same name
         // (as is done in UpdateListOfGlobals()),
         // this 'feature' is being used in TROOT::GetType().
         if (!d) {
            gROOT->fTypes->Add(new TDataType(new G__TypedefInfo(t)));
         }
         last_typenum = t.Typenum();
      }
   }
}

//______________________________________________________________________________
void TCint::SetClassInfo(TClass *cl, Bool_t reload)
{
   // Set pointer to CINT's G__ClassInfo in TClass.

   R__LOCKGUARD2(gCINTMutex);
   if (!cl->fClassInfo || reload) {

      delete cl->fClassInfo; cl->fClassInfo = 0;
      if (CheckClassInfo(cl->GetName())) {

         cl->fClassInfo = new G__ClassInfo(cl->GetName());

         // In case a class contains an external enum, the enum will be seen as a
         // class. We must detect this special case and make the class a Zombie.
         // Here we assume that a class has at least one method.
         // We can NOT call TClass::Property from here, because this method
         // assumes that the TClass is well formed to do a lot of information
         // caching. The method SetClassInfo (i.e. here) is usually called during
         // the building phase of the TClass, hence it is NOT well formed yet.
         if (cl->fClassInfo->IsValid() &&
             !(cl->fClassInfo->Property() & (kIsClass|kIsStruct|kIsNamespace))) {
            cl->MakeZombie();
         }

         if (!cl->fClassInfo->IsLoaded()) {
            if (cl->fClassInfo->Property() & (kIsNamespace)) {
               // Namespace can have a ClassInfo but no CINT dictionary per se
               // because they are auto-created if one of their contained
               // classes has a dictionary.
               cl->MakeZombie();
            }

            // this happens when no CINT dictionary is available
            delete cl->fClassInfo;
            cl->fClassInfo = 0;
         }

      }
   }
}

//______________________________________________________________________________
Bool_t TCint::CheckClassInfo(const char *name)
{
   // Checks if a class with the specified name is defined in CINT.
   // Returns kFALSE is class is not defined.

   // In the case where the class is not loaded and belongs to a namespace
   // or is nested, looking for the full class name is outputing a lots of
   // (expected) error messages.  Currently the only way to avoid this is to
   // specifically check that each level of nesting is already loaded.
   // In case of templates the idea is that everything between the outer
   // '<' and '>' has to be skipped, e.g.: aap<pipo<noot>::klaas>::a_class

   char *classname = new char[strlen(name)*2];
   strcpy(classname,name);

   char *current = classname;
   while (*current) {

      while (*current && *current != ':' && *current != '<')
         current++;

      if (!*current) break;

      if (*current == '<') {
         int level = 1;
         current++;
         while (*current && level > 0) {
            if (*current == '<') level++;
            if (*current == '>') level--;
            current++;
         }
         continue;
      }

      // *current == ':', must be a "::"
      if (*(current+1) != ':') {
         Error("CheckClassInfo", "unexpected token : in %s", classname);
         delete [] classname;
         return kFALSE;
      }

      *current = '\0';
      G__ClassInfo info(classname);
      if (!info.IsValid()) {
         delete [] classname;
         return kFALSE;
      }
      *current = ':';
      current += 2;
   }
   strcpy(classname,name);

   Int_t tagnum = G__defined_tagname(classname, 2); // This function might modify the name (to add space between >>).
   if (tagnum >= 0) {
      delete [] classname;
      return kTRUE;
   }
   G__TypedefInfo t(name);
   if (t.IsValid() && !(t.Property()&G__BIT_ISFUNDAMENTAL)) {
      delete [] classname;
      return kTRUE;
   }

   delete [] classname;
   return kFALSE;
}

//______________________________________________________________________________
void TCint::CreateListOfBaseClasses(TClass *cl)
{
   // Create list of pointers to base class(es) for TClass cl.

   R__LOCKGUARD2(gCINTMutex);
   if (!cl->fBase) {

      cl->fBase = new TList;

      G__BaseClassInfo t(*cl->GetClassInfo()), *a;
      while (t.Next()) {
         // if name cannot be obtained no use to put in list
         if (t.IsValid() && t.Name()) {
            a = new G__BaseClassInfo(t);
            cl->fBase->Add(new TBaseClass(a, cl));
         }
      }
   }
}

//______________________________________________________________________________
void TCint::CreateListOfDataMembers(TClass *cl)
{
   // Create list of pointers to data members for TClass cl.

   R__LOCKGUARD2(gCINTMutex);
   if (!cl->fData) {

      cl->fData = new TList;

      G__DataMemberInfo t(*cl->GetClassInfo()), *a;
      while (t.Next()) {
         // if name cannot be obtained no use to put in list
         if (t.IsValid() && t.Name() && strcmp(t.Name(), "G__virtualinfo")) {
            a = new G__DataMemberInfo(t);
            cl->fData->Add(new TDataMember(a, cl));
         }
      }
   }
}

//______________________________________________________________________________
void TCint::CreateListOfMethods(TClass *cl)
{
   // Create list of pointers to methods for TClass cl.

   R__LOCKGUARD2(gCINTMutex);
   if (!cl->fMethod) {

      cl->fMethod = new TList;

      G__MethodInfo t(*cl->GetClassInfo()), *a;
      while (t.Next()) {
         // if name cannot be obtained no use to put in list
         if (t.IsValid() && t.Name()) {
            a = new G__MethodInfo(t);
            cl->fMethod->Add(new TMethod(a, cl));
         }
      }
   }
}

//______________________________________________________________________________
void TCint::CreateListOfMethodArgs(TFunction *m)
{
   // Create list of pointers to method arguments for TMethod m.

   R__LOCKGUARD2(gCINTMutex);
   if (!m->fMethodArgs) {

      m->fMethodArgs = new TList;

      G__MethodArgInfo t(*m->fInfo), *a;
      while (t.Next()) {
         // if type cannot be obtained no use to put in list
         if (t.IsValid() && t.Type()) {
            a = new G__MethodArgInfo(t);
            m->fMethodArgs->Add(new TMethodArg(a, m));
         }
      }
   }
}

//______________________________________________________________________________
TString TCint::GetMangledName(TClass *cl, const char *method,
                             const char *params)
{
   // Return the CINT mangled name for a method of a class with parameters
   // params (params is a string of actual arguments, not formal ones). If the
   // class is 0 the global function list will be searched.

   R__LOCKGUARD2(gCINTMutex);
   G__CallFunc  func;
   Long_t       offset;

   if (cl)
      func.SetFunc(cl->GetClassInfo(), method, params, &offset);
   else {
      G__ClassInfo gcl;   // default G__ClassInfo is global environment
      func.SetFunc(&gcl, method, params, &offset);
   }
   return func.GetMethodInfo().GetMangledName();
}

//______________________________________________________________________________
TString TCint::GetMangledNameWithPrototype(TClass *cl, const char *method,
                                           const char *proto)
{
   // Return the CINT mangled name for a method of a class with a certain
   // prototype, i.e. "char*,int,float". If the class is 0 the global function
   // list will be searched.

   R__LOCKGUARD2(gCINTMutex);
   Long_t             offset;

   if (cl)
      return cl->GetClassInfo()->GetMethod(method, proto, &offset).GetMangledName();
   G__ClassInfo gcl;   // default G__ClassInfo is global environment
   return gcl.GetMethod(method, proto, &offset).GetMangledName();
}

//______________________________________________________________________________
void *TCint::GetInterfaceMethod(TClass *cl, const char *method,
                                const char *params)
{
   // Return pointer to CINT interface function for a method of a class with
   // parameters params (params is a string of actual arguments, not formal
   // ones). If the class is 0 the global function list will be searched.

   R__LOCKGUARD2(gCINTMutex);
   G__CallFunc  func;
   Long_t       offset;

   if (cl)
      func.SetFunc(cl->GetClassInfo(), method, params, &offset);
   else {
      G__ClassInfo gcl;   // default G__ClassInfo is global environment
      func.SetFunc(&gcl, method, params, &offset);
   }
   return (void *)func.InterfaceMethod();
}

//______________________________________________________________________________
void *TCint::GetInterfaceMethodWithPrototype(TClass *cl, const char *method,
                                             const char *proto)
{
   // Return pointer to CINT interface function for a method of a class with
   // a certain prototype, i.e. "char*,int,float". If the class is 0 the global
   // function list will be searched.

   R__LOCKGUARD2(gCINTMutex);
   G__InterfaceMethod f;
   Long_t             offset;

   if (cl)
      f = cl->GetClassInfo()->GetMethod(method, proto, &offset).InterfaceMethod();
   else {
      G__ClassInfo gcl;   // default G__ClassInfo is global environment
      f = gcl.GetMethod(method, proto, &offset).InterfaceMethod();
   }
   return (void *)f;
}

//______________________________________________________________________________
const char *TCint::GetInterpreterTypeName(const char *name, Bool_t full)
{
   // The 'name' is known to the interpreter, this function returns
   // the internal version of this name (usually just resolving typedefs)
   // This is used in particular to synchronize between the name used
   // by rootcint and by the run-time enviroment (TClass)
   // Return 0 if the name is not known.

   if (!gInterpreter->CheckClassInfo(name)) return 0;
   G__ClassInfo cl(name);
   if (cl.IsValid()) {
      if (full) return cl.Fullname();
      else return cl.Name();
   }
   else return 0;
}

//______________________________________________________________________________
void TCint::Execute(const char *function, const char *params, int *error)
{
   // Execute a global function with arguments params.

   R__LOCKGUARD2(gCINTMutex);
   G__CallFunc  func;
   G__ClassInfo cl;
   Long_t       offset;

   // set pointer to interface method and arguments
   func.SetFunc(&cl, function, params, &offset);

   // call function
   func.Exec(0);
   if (error) *error = G__lasterror();
}

//______________________________________________________________________________
void TCint::Execute(TObject *obj, TClass *cl, const char *method,
                    const char *params, int *error)
{
   // Execute a method from class cl with arguments params.

   R__LOCKGUARD2(gCINTMutex);
   void       *address;
   Long_t      offset;
   G__CallFunc func;

   // If the actuall class of this object inherit 2nd (or more) from TObject,
   // 'obj' is unlikely to be the start of the object (as described by IsA()),
   // hence gInterpreter->Execute will improperly correct the offset.

   void *addr = cl->DynamicCast( TObject::Class(), obj, kFALSE);

   // set pointer to interface method and arguments
   func.SetFunc(cl->GetClassInfo(), method, params, &offset);

   // call function
   address = (void*)((Long_t)addr + offset);
   func.Exec(address);
   if (error) *error = G__lasterror();
}

//______________________________________________________________________________
void TCint::Execute(TObject *obj, TClass *cl, TMethod *method, TObjArray *params,
                    int *error)
{
   // Execute a method from class cl with the arguments in array params
   // (params[0] ... params[n] = array of TObjString parameters).

   // Convert the TObjArray array of TObjString parameters to a character
   // string of comma separated parameters.
   // The parameters of type 'char' are enclosed in double quotes and all
   // internal quotes are escaped.

   if (!method) {
      Error("Execute","No method was defined");
      return;
   }

   TList *argList = method->GetListOfMethodArgs();

   // Check number of actual parameters against of expected formal ones

   Int_t nparms = argList->LastIndex()+1;
   Int_t argc   = params ? params->LastIndex()+1:0;

   if (nparms != argc) {
      Error("Execute","Wrong number of the parameters");
      return;
   }

   const char *listpar = "";
   TString complete(10);

   if (params)
   {
      // Create a character string of parameters from TObjArray
      TIter next(params);
      for (Int_t i = 0; i < argc; i ++)
      {
         TMethodArg *arg = (TMethodArg *) argList->At( i );
         G__TypeInfo type( arg->GetFullTypeName() );
         TObjString *nxtpar = (TObjString *)next();
         if (i) complete += ',';
         if (strstr( type.TrueName(), "char" )) {
            TString chpar('\"');
            chpar += (nxtpar->String()).ReplaceAll("\"","\\\"");
            // At this point we have to check if string contains \\"
            // and apply some more sophisticated parser. Not implemented yet!
            complete += chpar;
            complete += '\"';
         }
         else
            complete += nxtpar->String();
      }
      listpar = complete.Data();
   }

   Execute(obj, cl, (char *)method->GetName(), (char *)listpar, error);
}

//______________________________________________________________________________
Long_t TCint::ExecuteMacro(const char *filename, EErrorCode *error)
{
   // Execute a CINT macro.

   return TApplication::ExecuteFile(filename, (int*)error);
}

//______________________________________________________________________________
const char *TCint::GetTopLevelMacroName()
{
   // Return the file name of the current un-included interpreted file.
   // See the documentation for GetCurrentMacroName().

   G__SourceFileInfo srcfile(G__get_ifile()->filenum);
   while (srcfile.IncludedFrom().IsValid())
      srcfile = srcfile.IncludedFrom();

   return srcfile.Name();
}

//______________________________________________________________________________
const char *TCint::GetCurrentMacroName()
{
   // Return the file name of the currently interpreted file,
   // included or not. Example to illustrate the difference between
   // GetCurrentMacroName() and GetTopLevelMacroName():
   // BEGIN_HTML <!--
   /* -->
      <span style="color:#ffffff;background-color:#7777ff;padding-left:0.3em;padding-right:0.3em">inclfile.h</span>
      <!--div style="border:solid 1px #ffff77;background-color: #ffffdd;float:left;padding:0.5em;margin-bottom:0.7em;"-->
      <div class="code">
      <pre style="margin:0pt">#include &lt;iostream&gt;
void inclfunc() {
   std::cout &lt;&lt; "In inclfile.h" &lt;&lt; std::endl;
   std::cout &lt;&lt; "  TCint::GetCurrentMacroName() returns  " &lt;&lt;
      TCint::GetCurrentMacroName() &lt;&lt; std::endl;
   std::cout &lt;&lt; "  TCint::GetTopLevelMacroName() returns " &lt;&lt;
      TCint::GetTopLevelMacroName() &lt;&lt; std::endl;
}</pre></div>
      <div style="clear:both"></div>
      <span style="color:#ffffff;background-color:#7777ff;padding-left:0.3em;padding-right:0.3em">mymacro.C</span>
      <div style="border:solid 1px #ffff77;background-color: #ffffdd;float:left;padding:0.5em;margin-bottom:0.7em;">
      <pre style="margin:0pt">#include &lt;iostream&gt;
#include "inclfile.h"
void mymacro() {
   std::cout &lt;&lt; "In mymacro.C" &lt;&lt; std::endl;
   std::cout &lt;&lt; "  TCint::GetCurrentMacroName() returns  " &lt;&lt;
      TCint::GetCurrentMacroName() &lt;&lt; std::endl;
   std::cout &lt;&lt; "  TCint::GetTopLevelMacroName() returns " &lt;&lt;
      TCint::GetTopLevelMacroName() &lt;&lt; std::endl;
   std::cout &lt;&lt; "  Now calling inclfunc..." &lt;&lt; std::endl;
   inclfunc();
}</pre></div>
<div style="clear:both"></div>
<!-- */
// --> END_HTML
   // Running mymacro.C will print:
   //
   // root [0] .x mymacro.C
   // In mymacro.C
   //   TCint::GetCurrentMacroName() returns  ./mymacro.C
   //   TCint::GetTopLevelMacroName() returns ./mymacro.C
   //   Now calling inclfunc...
   // In inclfile.h
   //   TCint::GetCurrentMacroName() returns  inclfile.h
   //   TCint::GetTopLevelMacroName() returns ./mymacro.C

   return G__get_ifile()->name;
}


//______________________________________________________________________________
const char *TCint::TypeName(const char *typeDesc)
{
   // Return the absolute type of typeDesc.
   // E.g.: typeDesc = "class TNamed**", returns "TNamed".
   // You need to use the result immediately before it is being overwritten.

   static char t[1024];
   char *s, *template_start;
   if (!strstr(typeDesc, "(*)(")) {
      s = (char*)strchr(typeDesc, ' ');
      template_start = (char*)strchr(typeDesc, '<');
      if (!strcmp(typeDesc, "long long"))
         strcpy(t, typeDesc);
      else if (!strncmp(typeDesc,"unsigned ",s+1-typeDesc))
         strcpy(t, typeDesc);
      // s is the position of the second 'word' (if any)
      // except in the case of templates where there will be a space
      // just before any closing '>': eg.
      //    TObj<std::vector<UShort_t,__malloc_alloc_template<0> > >*
      else if (s && (template_start==0 || (s < template_start)) )
         strcpy(t, s+1);
      else
         strcpy(t, typeDesc);
   }

   int l = strlen(t);
   while (l > 0 && (t[l-1] == '*' || t[l-1] == '&') ) t[--l] = 0;

   return t;
}

//______________________________________________________________________________
Int_t TCint::LoadLibraryMap(const char *rootmapfile)
{
   // Load map between class and library. If rootmapfile is specified a
   // specific rootmap file can be added (typically used by ACLiC).
   // In case of error -1 is returned, 0 otherwise.
   // Cint uses this information to automatically load the shared library
   // for a class (autoload mechanism).
   // See also the AutoLoadCallback() method below.

   // open the [system].rootmap files
   if (!fMapfile) {
      fMapfile = new TEnv(".rootmap");
      fMapfile->IgnoreDuplicates(kTRUE);

      fRootMapFiles = new TObjArray;
      fRootMapFiles->SetOwner();

      // Load all rootmap files in the dynamic load path ((DY)LD_LIBRARY_PATH, etc.).
      // A rootmap file must end with the string ".rootmap".
      TString ldpath = gSystem->GetDynamicPath();
#ifdef WIN32
      TObjArray *paths = ldpath.Tokenize(";");
#else
      TObjArray *paths = ldpath.Tokenize(":");
#endif

      TString d;
      for (Int_t i = 0; i < paths->GetEntriesFast(); i++) {
         d = ((TObjString*)paths->At(i))->GetString();
         // check if directory already scanned
         Int_t skip = 0;
         for (Int_t j = 0; j < i; j++) {
            TString pd = ((TObjString*)paths->At(j))->GetString();
            if (pd == d) {
               skip++;
               break;
            }
         }
         if (!skip) {
            void *dirp = gSystem->OpenDirectory(d);
            if (dirp) {
               if (gDebug > 0)
                  Info("LoadLibraryMap", "%s", d.Data());
               const char *f1;
               while ((f1 = gSystem->GetDirEntry(dirp))) {
                  TString f = f1;
                  if (f.EndsWith(".rootmap")) {
                     TString p;
                     p = d + "/" + f;
                     if (!gSystem->AccessPathName(p, kReadPermission)) {
                        if (gDebug > 1)
                           Info("LoadLibraryMap", "   rootmap file: %s", p.Data());
                        fMapfile->ReadFile(p, kEnvGlobal);
                        fRootMapFiles->Add(new TObjString(p));
                     }
                  }
                  if (f.BeginsWith("rootmap")) {
                     TString p;
                     p = d + "/" + f;
                     FileStat_t stat;
                     if (gSystem->GetPathInfo(p, stat) == 0 && R_ISREG(stat.fMode))
                        Warning("LoadLibraryMap", "please rename %s to end with \".rootmap\"", p.Data());
                  }
               }
            }
            gSystem->FreeDirectory(dirp);
         }
      }

      delete paths;
      if (!fMapfile->GetTable()->GetEntries()) {
         return -1;
      }
   }

   if (rootmapfile && *rootmapfile) {
      // Add content of a specific rootmap file
      fMapfile->IgnoreDuplicates(kFALSE);
      fMapfile->ReadFile(rootmapfile, kEnvGlobal);
   }

   TEnvRec *rec;
   TIter next(fMapfile->GetTable());

   while ((rec = (TEnvRec*) next())) {
      TString cls = rec->GetName();
      if (!strncmp(cls.Data(), "Library.", 8) && cls.Length() > 8) {

         // get the first lib from the list of lib and dependent libs
         TString libs = rec->GetValue();
         if (libs == "") continue;
         TString delim(" ");
         TObjArray *tokens = libs.Tokenize(delim);
         char *lib = (char *)((TObjString*)tokens->At(0))->GetName();
         // convert "@@" to "::", we used "@@" because TEnv
         // considers "::" a terminator
         cls.Remove(0,8);
         cls.ReplaceAll("@@", "::");
         // convert "-" to " ", since class names may have
         // blanks and TEnv considers a blank a terminator
         cls.ReplaceAll("-", " ");
         if (cls.Contains(":")) {
            // We have a namespace and we have to check it first
            int slen = cls.Length();
            for (int k = 0; k < slen; k++) {
               if (cls[k] == ':') {
                  if (k+1 >= slen || cls[k+1] != ':') {
                     // we expected another ':'
                     break;
                  }
                  if (k) {
                     TString base = cls(0, k);
                     if (base == "std") {
                        // std is not declared but is also ignored by CINT!
                        break;
                     } else {
                        // Only declared the namespace do not specify any library because
                        // the namespace might be spread over several libraries and we do not
                        // know (yet?) which one the user will need!
                        G__set_class_autoloading_table((char*)base.Data(), "");
                     }
                     ++k;
                  }
               } else if (cls[k] == '<') {
                  // We do not want to look at the namespace inside the template parameters!
                  break;
               }
            }
         }
         G__set_class_autoloading_table((char*)cls.Data(), lib);
         G__security_recover(stderr); // Ignore any error during this setting.
         if (gDebug > 2) {
            const char *wlib = gSystem->Which(gSystem->GetDynamicPath(), lib);
            Info("LoadLibraryMap", "class %s in %s", cls.Data(), wlib);
            delete [] wlib;
         }
         delete tokens;
      }
   }
   return 0;
}

//______________________________________________________________________________
Int_t TCint::UnloadLibraryMap(const char *library)
{
   // Unload library map entries coming from the specified library.
   // Returns -1 in case no entries for the specified library were found,
   // 0 otherwise.

   if (!fMapfile || !library || !*library)
      return 0;

   TEnvRec *rec;
   TIter next(fMapfile->GetTable());

   Int_t ret = 0;

   while ((rec = (TEnvRec*) next())) {
      TString cls = rec->GetName();
      if (!strncmp(cls.Data(), "Library.", 8) && cls.Length() > 8) {

         // get the first lib from the list of lib and dependent libs
         TString libs = rec->GetValue();
         if (libs == "") continue;
         TString delim(" ");
         TObjArray *tokens = libs.Tokenize(delim);
         const char *lib = ((TObjString*)tokens->At(0))->GetName();
         // convert "@@" to "::", we used "@@" because TEnv
         // considers "::" a terminator
         cls.Remove(0,8);
         cls.ReplaceAll("@@", "::");
         // convert "-" to " ", since class names may have
         // blanks and TEnv considers a blank a terminator
         cls.ReplaceAll("-", " ");
         if (cls.Contains(":")) {
            // We have a namespace and we have to check it first
            int slen = cls.Length();
            for (int k = 0; k < slen; k++) {
               if (cls[k] == ':') {
                  if (k+1 >= slen || cls[k+1] != ':') {
                     // we expected another ':'
                     break;
                  }
                  if (k) {
                     TString base = cls(0, k);
                     if (base == "std") {
                        // std is not declared but is also ignored by CINT!
                        break;
                     } else {
                        // Only declared the namespace do not specify any library because
                        // the namespace might be spread over several libraries and we do not
                        // know (yet?) which one the user will need!
                        //G__remove_from_class_autoloading_table((char*)base.Data());
                     }
                     ++k;
                  }
               } else if (cls[k] == '<') {
                  // We do not want to look at the namespace inside the template parameters!
                  break;
               }
            }
         }

         if (!strcmp(library, lib)) {
            if (fMapfile->GetTable()->Remove(rec) == 0) {
               Error("UnloadLibraryMap", "entry for <%s,%s> not found in library map table", cls.Data(), lib);
               ret = -1;
            }
         }

         G__set_class_autoloading_table((char*)cls.Data(), "");
         G__security_recover(stderr); // Ignore any error during this setting.
         delete tokens;
      }
   }

   return ret;
}

//______________________________________________________________________________
Int_t TCint::AutoLoad(const char *cls)
{
   // Load library containing specified class. Returns 0 in case of error
   // and 1 in case if success.

   Int_t status = 0;

   if (!gROOT || !gInterpreter) return status;

   // Prevent the recursion when the library dictionary are loaded.
   Int_t oldvalue = G__set_class_autoloading(0);

   // lookup class to find list of dependent libraries
   TString deplibs = gInterpreter->GetClassSharedLibs(cls);
   if (!deplibs.IsNull()) {
      TString delim(" ");
      TObjArray *tokens = deplibs.Tokenize(delim);
      for (Int_t i = tokens->GetEntriesFast()-1; i > 0; i--) {
         const char *deplib = ((TObjString*)tokens->At(i))->GetName();
         if (gROOT->LoadClass(cls, deplib) == 0) {
            if (gDebug > 0)
               ::Info("TCint::AutoLoad", "loaded dependent library %s for class %s",
                      deplib, cls);
         } else
            ::Error("TCint::AutoLoad", "failure loading dependent library %s for class %s",
                    deplib, cls);
      }
      const char *lib = ((TObjString*)tokens->At(0))->GetName();

      if (lib[0]) {
         if (gROOT->LoadClass(cls, lib) == 0) {
            if (gDebug > 0)
               ::Info("TCint::AutoLoad", "loaded library %s for class %s",
                      lib, cls);
            status = 1;

            // LF 02/05/07
            // This is ugly ugly but I dont know how to solve it yet.
            // The library TGX11 has types like XEvent that are seen in the library
            // like _XEvent so when we look for them in CInt it will complain.
            // And I cant do a typedef when ROOT starts because the class
            // TGX11 has not been loaded !!! so We have to de it just
            // after loading the class... (maybe we could modify the
            // the class to include such types over there?)
            //if(strcmp(cls, "TGX11")==0)
            //   MapDependantTypesX();
               
         } else
            ::Error("TCint::AutoLoad", "failure loading library %s for class %s",
                    lib, cls);
      }
      delete tokens;
   }

   G__set_class_autoloading(oldvalue);
   return status;
}

//______________________________________________________________________________
Int_t TCint::AutoLoadCallback(const char *cls, const char *lib)
{
   // Load library containing specified class. Returns 0 in case of error
   // and 1 in case if success.

   if (!gROOT || !gInterpreter || !cls || !lib) return 0;

   // calls to load libCore might come in the very beginning when libCore
   // dictionary is not fully loaded yet, ignore it since libCore is always
   // loaded
   if (strstr(lib, "libCore")) return 1;

   // lookup class to find list of dependent libraries
   TString deplibs = gInterpreter->GetClassSharedLibs(cls);
   if (!deplibs.IsNull()) {
      TString delim(" ");
      TObjArray *tokens = deplibs.Tokenize(delim);
      for (Int_t i = tokens->GetEntriesFast()-1; i > 0; i--) {
         const char *deplib = ((TObjString*)tokens->At(i))->GetName();
         if (gROOT->LoadClass(cls, deplib) == 0) {
            if (gDebug > 0)
               ::Info("TCint::AutoLoadCallback", "loaded dependent library %s for class %s",
                      deplib, cls);
         } else {
            ::Error("TCint::AutoLoadCallback", "failure loading dependent library %s for class %s",
                      deplib, cls);
         }
      }
      delete tokens;
   }

   if (lib[0]) {
      if (gROOT->LoadClass(cls, lib) == 0) {
         if (gDebug > 0)
            ::Info("TCint::AutoLoadCallback", "loaded library %s for class %s",
            lib, cls);
         return 1;
      } else {
         ::Error("TCint::AutoLoadCallback", "failure loading library %s for class %s",
         lib, cls);
      }
   }
   return 0;
}

//______________________________________________________________________________
void *TCint::FindSpecialObject(const char *item, G__ClassInfo *type,
                               void **prevObj, void **assocPtr)
{
   // Static function called by CINT when it finds an un-indentified object.
   // This function tries to find the UO in the ROOT files, directories, etc.
   // This functions has been registered by the TCint ctor.

   if (!*prevObj || *assocPtr != gDirectory) {
      *prevObj = gROOT->FindSpecialObject(item, *assocPtr);
      if (!fgSetOfSpecials) fgSetOfSpecials = new std::set<TObject*>;
      if (*prevObj) ((std::set<TObject*>*)fgSetOfSpecials)->insert((TObject*)*prevObj);
   }

   if (*prevObj) type->Init(((TObject *)*prevObj)->ClassName());
   return *prevObj;
}

//______________________________________________________________________________
// Helper class for UpdateClassInfo
namespace {
   class TInfoNode {
   private:
      string fName;
      Long_t fTagnum;
   public:
      TInfoNode(const char *item, Long_t tagnum)
         : fName(item),fTagnum(tagnum)
      {}
      void Update() {
         TCint::UpdateClassInfoWork(fName.c_str(),fTagnum);
      }
   };
}

//______________________________________________________________________________
void TCint::UpdateClassInfo(char *item, Long_t tagnum)
{
   // Static function called by CINT when it changes the tagnum for
   // a class (e.g. after re-executing the setup function). In such
   // cases we have to update the tagnum in the G__ClassInfo used by
   // the TClass for class "item".

   if (gROOT && gROOT->GetListOfClasses()) {

      static Bool_t entered = kFALSE;
      static vector<TInfoNode> updateList;
      Bool_t topLevel;

      if (entered) topLevel = kFALSE;
      else {
         entered = kTRUE;
         topLevel = kTRUE;
      }
      if (topLevel) {
         UpdateClassInfoWork(item,tagnum);
      } else {
         // If we are called indirectly from within another call to
         // TCint::UpdateClassInfo, we delay the update until the dictionary loading
         // is finished (i.e. when we return to the top level TCint::UpdateClassInfo).
         // This allows for the dictionary to be fully populated when we actually
         // update the TClass object.   The updating of the TClass sometimes
         // (STL containers and when there is an emulated class) forces the building
         // of the TClass object's real data (which needs the dictionary info).
         updateList.push_back(TInfoNode(item,tagnum));
      }
      if (topLevel) {
         while (!updateList.empty()) {
            TInfoNode current( updateList.back() );
            updateList.pop_back();
            current.Update();
         }
         entered = kFALSE;
      }
   }
}

//______________________________________________________________________________
void TCint::UpdateClassInfoWork(const char *item, Long_t tagnum)
{
   // This does the actual work of UpdateClassInfo.

   Bool_t load = kFALSE;
   if (strchr(item,'<') && TClass::fgClassShortTypedefHash) {
      // We have a template which may have duplicates.

      TString resolvedItem(
       TClassEdit::ResolveTypedef(TClassEdit::ShortType(item,
          TClassEdit::kDropStlDefault).c_str(), kTRUE) );

      if (resolvedItem != item) {
         TClass* cl= (TClass*)gROOT->GetListOfClasses()->FindObject(resolvedItem);
         if (cl)
            load = kTRUE;
      }

      if (!load) {
         TIter next(TClass::fgClassShortTypedefHash->GetListForObject(resolvedItem));

         while ( TClass::TNameMapNode* htmp =
              static_cast<TClass::TNameMapNode*> (next()) ) {
            if (resolvedItem == htmp->String()) {
               TClass* cl = gROOT->GetClass (htmp->fOrigName);
               if (cl) {
                  // we found at least one equivalent.
                  // let's force a reload
                  load = kTRUE;
                  break;
               }
            }
         }
      }
   }
  
   TClass *cl = gROOT->GetClass(item, load);
   if (cl) cl->ResetClassInfo(tagnum);
}


//______________________________________________________________________________
void TCint::UpdateAllCanvases()
{
   // Update all canvases at end the terminal input command.

   TIter next(gROOT->GetListOfCanvases());
   TVirtualPad *canvas;
   while ((canvas = (TVirtualPad *)next()))
      canvas->Update();
}

//______________________________________________________________________________
const char* TCint::GetSharedLibs()
{
   // Refresh the list of shared libraries and return it.

   fSharedLibs = "";

   G__SourceFileInfo cursor(0);
   while (cursor.IsValid()) {
      const char *filename = cursor.Name();
      if (filename==0) continue;
      Int_t len = strlen(filename);
      const char *end = filename+len;
      Bool_t needToSkip = kFALSE;
      if ( len>5 && (strcmp(end-4,".dll") == 0 ) ) {
         // Filter out the cintdlls
         static const char *excludelist [] = {
            "stdfunc.dll","stdcxxfunc.dll","posix.dll","ipc.dll","posix.dll"
            "string.dll","vector.dll","vectorbool.dll","list.dll","deque.dll",
            "map.dll", "map2.dll","set.dll","multimap.dll","multimap2.dll",
            "multiset.dll","stack.dll","queue.dll","valarray.dll",
            "exception.dll","stdexcept.dll","complex.dll","climits.dll"};
         for (unsigned int i=0; i < sizeof(excludelist)/sizeof(excludelist[0]); ++i) {
            if (strcmp(filename,excludelist[i])==0) { needToSkip = kTRUE; break; }
         }
      }
      if (!needToSkip &&
           ((len>3 && strcmp(end-2,".a") == 0)    ||
            (len>4 && (strcmp(end-3,".sl") == 0   ||
                       strcmp(end-3,".dl") == 0   ||
                       strcmp(end-3,".so") == 0)) ||
            (len>5 && (strcasecmp(end-4,".dll") == 0)))) {
         if (!fSharedLibs.IsNull())
            fSharedLibs.Append(" ");
         fSharedLibs.Append(filename);
      }

      cursor.Next();
   }

   return fSharedLibs;
}

//______________________________________________________________________________
const char *TCint::GetClassSharedLibs(const char *cls)
{
   // Get the list of shared libraries containing the code for class cls.
   // The first library in the list is the one containing the class, the
   // others are the libraries the first one depends on. Returns 0
   // in case the library is not found.

   if (!cls || !*cls)
      return 0;

   // lookup class to find list of libraries
   if (fMapfile) {
      TString c = TString("Library.") + cls;
      // convert "::" to "@@", we used "@@" because TEnv
      // considers "::" a terminator
      c.ReplaceAll("::", "@@");
      // convert "-" to " ", since class names may have
      // blanks and TEnv considers a blank a terminator
      c.ReplaceAll(" ", "-");
      const char *libs = fMapfile->GetValue(c, "");
      return (*libs) ? libs : 0;
   }
   return 0;
}

//______________________________________________________________________________
const char *TCint::GetSharedLibDeps(const char *lib)
{
   // Get the list a libraries on which the specified lib depends. The
   // returned string contains as first element the lib itself.
   // Returns 0 in case the lib does not exist or does not have
   // any dependencies.

   if (!fMapfile || !lib || !lib[0])
      return 0;

   TString libname(lib);
   Ssiz_t idx = libname.Last('.');
   if (idx != kNPOS) {
      libname.Remove(idx);
   }
   TEnvRec *rec;
   TIter next(fMapfile->GetTable());

   size_t len = libname.Length();
   while ((rec = (TEnvRec*) next())) {
      const char *libs = rec->GetValue();
      if (!strncmp(libs, libname.Data(), len) && strlen(libs) >= len
          && (!libs[len] || libs[len] == ' ' || libs[len] == '.')) {
         return libs;
      }
   }
   return 0;
}

//______________________________________________________________________________
Bool_t TCint::IsErrorMessagesEnabled() const
{
   // If error messages are disabled, the interpreter should suppress its
   // failures and warning messages from stdout.

   return !G__const_whatnoerror();
}

//______________________________________________________________________________
Bool_t TCint::SetErrorMessages(Bool_t enable)
{
   // If error messages are disabled, the interpreter should suppress its
   // failures and warning messages from stdout. Return the previous state.

   if (enable)
      G__const_resetnoerror();
   else
      G__const_setnoerror();
   return !G__const_whatnoerror();
}

//______________________________________________________________________________
void TCint::AddIncludePath(const char *path)
{
   // Add the given path to the list of directories in which the interpreter
   // looks for include files. Only one path item can be specified at a
   // time, i.e. "path1:path2" is not supported.

   char *incpath = gSystem->ExpandPathName(path);

   G__add_ipath(incpath);

   delete [] incpath;
}

//______________________________________________________________________________
const char *TCint::GetIncludePath()
{
   // Refresh the list of include paths known to the interpreter and return it
   // with -I prepended.

   fIncludePath = "";

   G__IncludePathInfo path;

   while (path.Next()) {
      const char *pathname = path.Name();
      fIncludePath.Append(" -I\"").Append(pathname).Append("\" ");
   }

   return fIncludePath;
}



//______________________________________________________________________________
//______________________________________________________________________________
// Methods used to register the function pointer (or method pointer)
// of a given signature inside CINT. This should probably go to another file
//
//______________________________________________________________________________
//______________________________________________________________________________

//______________________________________________________________________________
void TCint::MapDependantTypes()
{
   // There is problem with types ( well many :) ).
   // We can declare something like "FILE *fp" in our
   // normal ROOT classes but when gcc creates the symbols
   // it will use an internal type like "_IO_FILE".
   // This mapping is completely dependant of the compiler
   // so we have to be very careful about it.
   ProcessLine("typedef FILE _IO_FILE", 0);

   // gui/inc/WidgetMessageTypes.h
   // All the enums could be treated as ints
   ProcessLine("typedef int EWidgetMessageTypes", 0);

   ProcessLine("typedef void* istream", 0);
   ProcessLine("typedef void* istream&", 0);
   ProcessLine("typedef void* std::istream&", 0);

   ProcessLine("typedef int __false_type", 0);
   ProcessLine("typedef int __true_type", 0);

   //ProcessLine("typedef void ostream", 0);
   //ProcessLine("typedef void* ostream&", 0);
   //ProcessLine("typedef void* std::ostream&", 0);
}

//______________________________________________________________________________
void TCint::MapDependantTypesX()
{
   // Like MapDependantTypes but for the class TGX11

   ProcessLine("typedef struct _XDisplay  { } Display;", 0);
   ProcessLine("typedef struct _XGC       { } *GC;", 0);
   ProcessLine("typedef struct _XEvent    { } XEvent", 0);
   ProcessLine("typedef struct _XImage    { } Image", 0);
   ProcessLine("typedef struct            { } XFontStruct;", 0);
   ProcessLine("typedef struct FT_Bitmap_ { } FT_Bitmap", 0);
}

//______________________________________________________________________________
void TCint::MapDependantTypesTree()
{
   // dependant type in libTree.so
   
   // LF 07/05/2007
   // This class doesnt have a classdef so I dont know yet how to handle it
   //ProcessLine("typedef struct {} TTree::TFriendLock", 0);
   ProcessLine("typedef struct {} TFriendLock", 0);
   ProcessLine("typedef ifstream basic_ifstream<char, std::char_traits<char>>", 0);
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
   struct link_map *map = locate_linkmap();
   struct link_map *map_iter;
   struct link_map *map_res = 0;

   for (map_iter = map; map_iter; map_iter = map_iter->l_next) {
      //cerr << "map_iter->l_name: " << map_iter->l_name << endl;
      if( map_iter->l_name && 
          (strcmp(map_iter->l_name, libname)==0 ||
           strcmp(gSystem->BaseName(map_iter->l_name), gSystem->BaseName(libname))==0 )) {
         map_res = map_iter;
         break;
      }
   }

   free(map);
   return map_res;
}

//______________________________________________________________________________
static std::list<TSymbol*>*
// static TList*
SymbolList(const char* lib/*, int& ndict, int& nmem, int& nnonmem, int& ntot*/)
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

   /* Strings Table accessing */
   /* The symbol's names are in a separate table */
   char *strtab_pointer;

   for (Elf32_Dyn* dyn = map->l_ld; dyn->d_tag != DT_NULL; ++dyn)
      if (dyn->d_tag == DT_STRTAB)
         strtab_pointer = (char *) dyn->d_un.d_ptr;

   /* Symbol Table Entries Accessing */
   /* Symbols in ELF file are in the Table Entrie Structure */
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

   /* Hash table accessing */
   /* Finally we search the symbols in the Hash table*/
   /* The Hash table is a list of pointers to Symbol Entries structures */
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
         if ((sym_entry.st_value == 0) || (ELF32_ST_TYPE(sym_entry.st_info) != STT_FUNC)/* ||
                                          (ELF32_ST_BIND(sym_entry.st_info) == STB_WEAK) ||
                                          (ELF32_ST_BIND(sym_entry.st_info) == STB_NUM) */ ) 
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
   //*symbols = temp;
   //*address = tempAddress;

   return symbol_list;
}

//______________________________________________________________________________
void TCint::Register(const char *libname, const char *clstr)
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
   
   return;
   
   string classname;
   if(clstr)
      classname = clstr;

   UInt_t  classhash = hash(classname.c_str(), classname.size());
   Int_t nreg = 0;
   std::list<TSymbol*> *demangled = 0;
   Int_t nerrors = 0;
   Int_t isFreeFunc = classname.empty();
   Int_t destInCharge = 0;    // Was this desc registered?
   Int_t destInChargeDel = 0; // Was this desc registered?

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
                    !strcmp(libname+len-2, ".a"))) {
      if (gDebug > 0) {
         cerr << "****************************************" << endl;
         cerr << "Error: " << libname << " doesnt look like a valid library" << endl;
         cerr << "****************************************" << endl;
      }
      return;
   }


   const char *libpath = gSystem->ExpandPathName(libname);
   if (gDebug > 0) {
      cerr << "****************************************" << endl;
      cerr << "Reading symbols from library:" << libpath << endl;
   }


   /*********************************/
   // small hack to avoid funny types
   char *basec, *bname;
   basec = strdup(libname);
   bname = basename(basec);
   
   if(strcmp(bname, "libGX11.so")==0) {
      ((TCint*)gInterpreter)->MapDependantTypesX();
   }
   else if(strcmp(bname, "libTree.so")==0) {
      ((TCint*)gInterpreter)->MapDependantTypesTree();
   }
   free(basec);
   /*********************************/
   
   
   // LF: 09/05/07
   // Here we have the first try to implement the symbol cache.
   // The first thing to do is to look for this library in fSymbolTable
   // Note: Remember to keep all the libraries with their full path
   //TSymbolLookup *symt = (TSymbolLookup *) (((TCint*)gInterpreter)->GetSymbolTable())->find(libpath);
   multimap<string,TSymbolLookup*>           *symbolt = ((TCint*)gInterpreter)->GetSymbolTable();
   multimap<string,TSymbolLookup*>::iterator  iter    = symbolt->find(string(libpath));
   TSymbolLookup *symt = 0;
   if (iter != symbolt->end() ) {
         symt = (TSymbolLookup *) ((*iter).second);
   }

   //TSymbolLookup *symt = (TSymbolLookup *) ;
   if (gDebug > 0)
      cerr << "+++ Looking for the symbol table of: " << libpath << endl;

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

         delete [] libpath;
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

      demangled = SymbolList(libpath);

      //demangled = DeMangle(mangled);
      if (!demangled) {
         cerr << "Error reading/demangling the symbols" << endl;

         //delete mangled;
         delete [] libpath;
         return;
      }

      // Here we dont need the list with the mangled symbols anymore
      // so we can easily delete it
      //delete mangled;

      if (gDebug > 0)
         cerr << "+++ Add library '" << libpath << "' to the list of libraries" << endl << endl;

      // And now that we have the symbols here we add them to the cache (for future lookups)
      symt = new TSymbolLookup(libpath, demangled);
      (((TCint*)gInterpreter)->GetSymbolTable())->insert(make_pair(string(libpath), symt));

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
      string protostr = "";

      // 16/04/2007
      // We cant tokenize with () only because that spoils things
      // when we have the "operator()" like in:
      //  TMatrixT<double>::operator()(int, int)
      // This is extreamly annoying and proves that this whole
      // parsing section should be rewritten following a set of rules
      // instead of the piñata paradigm

      string::size_type icolon = sig.find("::");
      if (!isFreeFunc && (icolon != string::npos)) {
         // Dont split it with tokenize because the parameters can have things
         // like std::annoying
         classstr = string(sig, 0, icolon);
         protostr = sig.substr(icolon+2, sig.size() - (icolon+2));

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
      if(!clstr && protostr.find("operator")!=string::npos){
         ++list_iter;
         continue;
      }

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

      // LF: 14/05/07
      // Constructors and destructor are not as easy as we thought
      // So let's just ignore them for the moment (i.e. dont register them)
      //if ( classstr==methodstr){
      //   ++list_iter;
      //   continue;
      //}

      // If this is a destructor
      //string tilda = "~";
      //tilda += classstr;
      //if (tilda==methodstr){
      //   ++list_iter;
      //   continue;
      //}

      // DIEGO WARNING WARNING WARNING DIEGO AUUUUUUUUUUUUA!!! AUUUUUUUUAA!!
      // For the moment I don't want the Incharge delete destructor.
      if (symbol->fIsDest && symbol->fIsDestInCDel){
         ++list_iter;
         continue;
      }

      //if (symbol->fIsDest && symbol->fIsDestInCDel)
      //   destInChargeDel = 1;
      //else if(symbol->fIsDest && symbol->fIsDestInC) {
      //   destInCharge = 1;
         
         // If the in-charge deleting destructor was already
         // registered then dont register the in-charge (non deleting)
         // destructor
      //    if(destInChargeDel){
      //      ++list_iter;
      //      continue;      
      //   }
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
         
         int k = 0;
         while (pos != string::npos || lpos != string::npos) {
            // new token
            string paramtoken = signature.substr(lpos, pos - lpos);
            string newparam;
		

           if (!paramtoken.empty() && paramtoken.find("const")!=string::npos ) {
              string delim_param(" ");
              string singleparamold;

              // go to the first pos
              string::size_type lpos_param = paramtoken.find_first_not_of(delim_param, 0);
              // first delim
              string::size_type pos_param  = paramtoken.find_first_of(delim_param, lpos_param);
              if(pos_param == string::npos && lpos_param != pos_param)
                 pos_param = paramtoken.size();

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
      
            
      if(!symbol->fFunc){
         if (gDebug > 0)
            cerr << "xxx The address is 0. the method wont be registered" << endl << endl;
      }
      else if( ((TCint*)gInterpreter)->RegisterPointer(finalclass.c_str(), methodstr.c_str(), 
                                                       newsignature.c_str(), symbol->fFunc) == -1){
         // yahoo.... we can finally call our register method
         if (gDebug > 0) {
            cerr << "xxx Couldnt register the method: " << methodstr << endl;
            cerr << "xxx from the class    : " << finalclass << endl;
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

   delete [] libpath;
}

//______________________________________________________________________________
Int_t TCint::RegisterPointer(const char *classname, const char *method, const char *proto,
                             void *ptr)
{
   // Try to register the function pointer 'ptr' for a given method
   // of a given class with a certain prototype, i.e. "char*,int,float".
   // If the class is 0 the global function list will be searched (not yet).
   // Returns -1 if there was a problem and 0 if it was succesful

   R__LOCKGUARD2(gCINTMutex);
   G__CallFunc func;
   //G__InterfaceMethod f;
   G__MethodInfo m;
   //struct G__ifunc_table *ifunc = 0;
   Long_t offset;

   // This means it doesn't belog to a class
   // i.e. free standing
   if (!classname || strcmp(classname, "")==0) {
      //G__ifunc_table* iref = CallFunc::GetFunc(funcname,param,&offset);
      m = Cint::G__CallFunc::GetFunc(method,proto,&offset, Cint::G__CallFunc::ExactMatch);
   }
   else {
      G__ClassInfo cinfo(classname);

      // This means that the class (or namespace or whatever) doesn't exist in CInt
      if (!cinfo.IsValid())
         return -1;

      m = cinfo.GetMethod(method, proto, &offset, Cint::G__ClassInfo::ExactMatch,  
                          Cint::G__ClassInfo::WithInheritance, 2);
   }

   // set pointer to interface method and arguments
   func.SetFunc(m);

   // register the ptr function
   return func.SetFuncPtr(ptr);
}
