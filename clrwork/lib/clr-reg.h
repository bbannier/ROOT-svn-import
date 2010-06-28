#ifndef __CLR_REG_H__
#define __CLR_REG_H__

// Author: Zdenek Culik   16/04/2010

// reflex headers
#include "Reflex/Reflex.h"

// root headers
#include "TMap.h"

class TClrReg : public TObject {
public:
   static void Init();
private:
   TMap fDict;
public:
   TClrReg();
   void RegisterFunctionStub(const char* name, Reflex::StubFunction value);
   Reflex::StubFunction GetFunctionStub(const char* name);
};

extern TClrReg* gClrReg;

#endif // __CLR_REG_H__
