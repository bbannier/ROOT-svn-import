#include "TSymbolDependencyParser.h"
#include <iostream>

#if defined(__linux__)  || defined(__sun__)
# include "TElfSymbolExtractor.h"
typedef TElfFileIdent FileIdent_t; // ELF
#else
# error UNSUPPORTED ARCHITECTURE.
#endif


void TSymbolDependencyParser::Parse(std::list<const char* /*files*/>& neededLibs)
{
   // Scan the symbol dependencies; put needed libraries into neededLibs
   // in the order passed into TSymbolDependencyParser's constructor.
   typedef CPtrSet_t NeededLibs_t;
   typedef CPtrSet_t DefinedSyms_t;
   NeededLibs_t neededLibsSet;
   DefinedSyms_t definedSyms;

   ExtractSymbols();

   for (SymList_t::iterator iUndef = fUndefObjSyms.begin(), iUndefE = fUndefObjSyms.end();
        iUndef != iUndefE; ++iUndef) {
      // mark as resolved
      definedSyms.insert(*iUndef);
      // find lib / obj defining the symbol:
      DefSymMap_t::const_iterator iDefSym = fDefSyms.find(*iUndef);
      // whether object or not known. If not known assume it's our fault and don't complain.
      
      if (iDefSym != fDefSyms.end()) {
         const char* resolving = iDefSym->second;
         if (resolving) {
            // It's a library.
            if (neededLibsSet.find(resolving) == neededLibsSet.end()) {
               // not yet depending on this lib, so add it.
               // std::cout << *iUndef << " needs " << resolving << std::endl;
               neededLibsSet.insert(resolving);
               std::map<const char*, SymList_t >::const_iterator iLibSym = fUndefLibSyms.find(resolving);
               if (iLibSym != fUndefLibSyms.end()) {
                  for (SymList_t::const_iterator iUndefLibSym = iLibSym->second.begin(),
                          iUndefLibSymE = iLibSym->second.end(); iUndefLibSym != iUndefLibSymE; ++iUndefLibSym) {
                     if (definedSyms.find(*iUndefLibSym) == definedSyms.end()) {
                        fUndefObjSyms.push_back(*iUndefLibSym);
                     }
                  }
               }
            }
         }
      }
      /*
      else {
         std::cerr << "SYMBOL " << *iUndef << " NOT FOUND!" << std::endl;
      }
      */
   }

   neededLibs.clear();
   // Keep fAllFiles' order
   for (std::list<const char*>::const_iterator iLib = fAllFiles.begin(),
           iLibE = fAllFiles.end(); iLib != iLibE; ++iLib) {
      if (neededLibsSet.find(*iLib) != neededLibsSet.end())
         neededLibs.push_back(*iLib);
   }
}

void TSymbolDependencyParser::ExtractSymbols()
{
   TSymbolDependencyParser::DefSymMap_t defObjSyms; // not used

   for (std::list<const char*>::const_iterator iLib = fAllFiles.begin(),
           iLibE = fAllFiles.end(); iLib != iLibE; ++iLib) {
      FileIdent_t *id = 0;
      try {
         id = new FileIdent_t(*iLib);
      }
      catch(...) {
         delete id;
         continue;
      }
      fIds.push_back(id);
      if (id->Type() == TFileIdent::kObject) {
         id->GetSymbols(defObjSyms, UndefObjSyms());
      } else {
         id->GetSymbols(DefSyms(), UndefLibSyms()[id->Name()]);
      }
      defObjSyms.clear();
   }
}

TSymbolDependencyParser::~TSymbolDependencyParser()
{
   for (std::list<TFileIdent*>::iterator iId = fIds.begin(), iIde = fIds.end();
        iId != iIde; ++iId)
      delete *iId;
}

