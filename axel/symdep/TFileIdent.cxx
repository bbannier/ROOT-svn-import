#include "TFileIdent.h"
#include <iostream>
#include <stdexcept>
#include <cstdio>
#include <cstdlib>

TFileIdent::TFileIdent(const char* name, EType type):
   fFileName(name), fType(type), fFd(0) {
   fFd = fopen(name, "r");
   if (!fFd) {
      Fatal("cannot be found.");
   }
}

TFileIdent::~TFileIdent() {
   if (fFd) fclose(fFd);
}

bool TFileIdent::IsLSB() const {
   static long l = 1;
   return (*((char*)&l));
}

void TFileIdent::Fatal(const char* msg) const {
   std::cerr << "File " << Name() << " " << msg << std::endl;
   throw std::runtime_error(msg);
}

