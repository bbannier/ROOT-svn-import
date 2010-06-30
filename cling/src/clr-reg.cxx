// Author: Zdenek Culik   16/04/2010

#include "clr-reg.h"

// FIXME: Need include for Reflex::StubFunction here!!!
#include "TObjString.h"

TClrReg* gClrReg = 0;

void TClrReg::Init()
{
   if (!gClrReg) {
      gClrReg = new TClrReg();
   }
}

TClrReg::TClrReg()
{
}

class TRegEntry : public TObject {
public:
   Reflex::StubFunction fValue;
};

void TClrReg::RegisterFunctionStub(const char* name, Reflex::StubFunction value)
{
   if (fDict.GetValue(name) != 0) {
   }
   TObjString* key = new TObjString(name);
   TRegEntry* obj = new TRegEntry;
   obj->fValue = value;
   fDict.Add(key, obj);
   if (fDict.GetValue(name) != obj) {
   }
}

Reflex::StubFunction TClrReg::GetFunctionStub(const char* name)
{
   TRegEntry* obj = (TRegEntry*) fDict.GetValue(name);
   if (!obj) {
      return 0;
   }
   return obj->fValue;
}

