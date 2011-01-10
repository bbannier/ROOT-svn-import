// @(#)root/cint:$Id$
// Author: Zdenek Culik   16/04/2010

#include "clr-reg.h"

TClrReg* gClrReg = 0;

//______________________________________________________________________________
void TClrReg::Init()
{
   if (!gClrReg) {
      gClrReg = new TClrReg();
   }
}

//______________________________________________________________________________
TClrReg::TClrReg()
{
   fFirstContext = NULL;
   fLastContext = NULL;
}

//______________________________________________________________________________
void TClrReg::ResetContexts ()
{
   fFirstContext = NULL;
   fLastContext = NULL;
}

//______________________________________________________________________________
void TClrReg::LinkContext (TClrContext * context)
{
   if (fLastContext == NULL)
      fFirstContext = context;
   else
      fLastContext->next = context;

   fLastContext = context;

   context->next = NULL;
}

//______________________________________________________________________________
void TClrReg::DeleteContexts ()
{
   TClrContext* p = fFirstContext;
   while (p != NULL)
   {
      TClrContext* t = p;
      p = p->next;
      delete t;
   }

   fFirstContext = NULL;
   fLastContext = NULL;
}

//______________________________________________________________________________
void TClrReg::GetFunctionStub(const std::string& func_name,
                              Reflex::StubFunction& stub,
                              void*& context)
{
   // create function stub and context
   // for stroring into Reflex dictionary when Reflex information is scanned

   TClrContext * c  = new TClrContext;
   // DO NOT LinkContext (c);
   c->name = func_name;

   stub = ClrStubFunction;
   context = c;
}

//______________________________________________________________________________
void ClrStubFunction (void* result, void* obj, const std::vector<void*>& params, void* ctx)
{
   TClrContext* context = reinterpret_cast <TClrContext *> (ctx);
   assert (context != NULL);
   std::cerr << "CLR clr-reg:   ClrStubFunction: Calling stub function " << context->index << ", " << context->name << std::endl;

   // TO DO: call LLVM function context->func
}

//______________________________________________________________________________
void gAddWrapperMap (std::map<std::string, Reflex::StubFunction> & wrappers)
{
   gClrReg->AddWrapperMap (wrappers);
}

