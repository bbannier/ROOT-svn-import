
#include "UnitTest.h"
#include <iostream>
#include <iomanip>
#include "TString.h"
#include <typeinfo>

using namespace std;
using namespace UnitTesting;

void UnitTest::do_test(bool cond, const std::string& lbl, const char* fname, long lineno)
{
   if (!cond)
      do_fail(lbl, fname, lineno);
   else
      succeed_();
}

void UnitTest::do_fail(const std::string& lbl, const char* fname, long lineno)
{
   ++nFail;
   if (osptr)
      {
         *osptr << "failure: " << setw(76) << std::left << lbl << std::right << " [line " << setw(3) << lineno << "]\n";
      }
}

const std::string & UnitTest::name() const
{
   if(fName=="")
      fName=std::string(typeid(*this).name());
   return fName;
}


long UnitTest::report() const
{
   if (osptr)
      {
         std::string counts(Form(" [%i/%i]", nPass, nPass+nFail));

         *osptr << name() << counts;

         UInt_t ndots = 82-name().size() - counts.size();

         for (UInt_t i=0; i<ndots; ++i) *osptr << '.';

         *osptr << (nFail==0?"..OK":"FAIL") << endl;
      }
   return nFail;
}

void UnitTest::intro() const
{
   if (osptr) {
      *osptr << "************************************************************************************************" << endl;
      *osptr << "* Starting U N I T test : " << name() << " (file " << fFileName << ")" << endl;
      *osptr << "************************************************************************************************" << endl;
   }
}
