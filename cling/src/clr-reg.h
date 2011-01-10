// @(#)root/cint:$Id$
// Author: Zdenek Culik   16/04/2010

#ifndef __CLR_REG_H__
#define __CLR_REG_H__

// reflex headers
#include "Reflex/Reflex.h"

#include "llvm/Module.h" // class llvm::Function

/* -------------------------------------------------------------------------- */

void ClrStubFunction (void* result, void* obj, const std::vector<void*>& params, void* ctx);

struct TClrContext
{
   const llvm::Function * func;

   // Reflex::Member* member;
   std::string name; // only for debugging

   int index;
   TClrContext * next;

   TClrContext () :
      func (NULL),
      // member (NULL),
      index (0),
      next (NULL)
      { }
};

/* -------------------------------------------------------------------------- */

class TClrReg {

private:
   typedef std::map<std::string, Reflex::StubFunction> WrapperMap_t;
   WrapperMap_t fWrappers;

public:
   void AddWrapperMap (WrapperMap_t wrappers) { fWrappers = wrappers; }

public:
   void GetFunctionStub(const std::string& func_name,
                        Reflex::StubFunction& stub,
                        void*& context);

private:
   TClrContext* fFirstContext;
   TClrContext* fLastContext;

public:
   void ResetContexts ();
   void LinkContext (TClrContext * context);
   void DeleteContexts ();
   TClrContext* GetFirstContext () { return fFirstContext; }

private:
   TClrReg();

public:
   static void Init();
};

/* -------------------------------------------------------------------------- */

extern TClrReg* gClrReg;

void gAddWrapperMap (std::map<std::string, Reflex::StubFunction> & wrappers);

/* -------------------------------------------------------------------------- */

#endif // __CLR_REG_H__
