#ifndef INCLUDE_TFILEIDENT
#define INCLUDE_TFILEIDENT

#include <list>
#include "TSymbolDependencyParser.h"

class TSymbolExtractor;

class TFileIdent {
public:
   enum EType {
      kObject,
      kSharedLib,
      kOther,
      kNumTypes
   };
   typedef unsigned char UChar_t;

   TFileIdent(const char* name, EType type);

   virtual ~TFileIdent();

   virtual void GetSymbols(TSymbolDependencyParser::DefSymMap_t& def,
                           std::list<const char*>& undef) = 0;

   const char* Name() const { return fFileName; }
   EType Type() const { return fType; }

   bool IsLSB() const;

   FILE* Fd() const { return fFd; }

   void Fatal(const char* msg) const;

protected:
   const char* fFileName; // library name
   EType fType; // file type
   FILE* fFd;
   mutable TSymbolExtractor* fExtractor;
};

#endif
