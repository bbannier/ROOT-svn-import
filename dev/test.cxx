
#include <string>
#include "test.h"

#ifndef __CINT__
#include "RooGlobalFunc.h"
#endif
#include "TRandom.h"
#include "TBenchmark.h"
#include "RooRealVar.h"
#include "RooBernstein.h"
#include "RooStringVar.h"
#include "TMath.h"

using namespace RooFit;

A::A()
{
   field = 0;
}

A::A(const A& rhs)
{
   field = rhs.field;
}


int main(int argc, char *argv[]) {

   A a;

   return 0;

}

