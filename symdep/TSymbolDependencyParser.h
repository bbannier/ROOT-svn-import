#ifndef INCLUDE_TSYMBOLDEPENDENCYPARSER
#define INCLUDE_TSYMBOLDEPENDENCYPARSER

#include "hashcont.h"
#include <list>
#include <map>

class TFileIdent;

class TSymbolDependencyParser {
public:
   typedef CPtrCPtrMap_t DefSymMap_t;
   typedef std::list<const char*> SymList_t;

   TSymbolDependencyParser(const std::list<const char*>& libs):
      fAllFiles(libs) {}
   ~TSymbolDependencyParser();

   SymList_t& UndefObjSyms() { return fUndefObjSyms; }
   DefSymMap_t& DefSyms() { return fDefSyms; }
   std::map<const char* /*file*/, SymList_t>& UndefLibSyms() {
      return fUndefLibSyms;
   }

   void Parse(std::list<const char* /*files*/>& neededLibs);

private:
   void ExtractSymbols();

   SymList_t fUndefObjSyms;
   DefSymMap_t fDefSyms;
   std::map<const char* /*file*/, SymList_t> fUndefLibSyms;
   std::list<const char*> fAllFiles;
   std::list<TFileIdent*> fIds;
};

#endif // INCLUDE_TSYMBOLDEPENDENCYPARSER
