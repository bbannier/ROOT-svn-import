#include "UnitTest.h"
#include <iostream>
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
      *osptr << typeid(*this).name() << "failure: (" << lbl << ") , "
	     << fname << " (line " << lineno << ")\n";
    }
}

long UnitTest::report() const 
{
  if (osptr)
    {
      *osptr << "UnitTest \"" << typeid(*this).name() << "\":\n\tPassed: " << nPass
	     << "\tFailed: " << nFail << endl;
    }
  return nFail;
}
