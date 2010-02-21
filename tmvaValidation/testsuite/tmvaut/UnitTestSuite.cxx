#include "UnitTestSuite.h"
#include <iostream>
#include <iomanip>
#include <cassert>
using namespace std;
using namespace UnitTesting;

void UnitTestSuite::addTest(UnitTest* t) throw(UnitTestSuiteError)
{
   // Verify test is valid and has a stream:
   if (t == 0)
      throw UnitTestSuiteError("Null test in UnitTestSuite::addTest");
   else if (osptr && !t->getStream())
      t->setStream(osptr);
   tests.push_back(t);
   t->reset();
}

void UnitTestSuite::addSuite(const UnitTestSuite& s)
{
   for (size_t i = 0; i < s.tests.size(); ++i)
      {
         assert(tests[i]);
         addTest(s.tests[i]);
      }
}

void UnitTestSuite::free()
{
   for (size_t i = 0; i < tests.size(); ++i)
      {
         delete tests[i];
         tests[i] = 0;
      }
}

void UnitTestSuite::run()
{
   reset();
   for (size_t i = 0; i < tests.size(); ++i)
      {
         assert(tests[i]);
         tests[i]->intro();
         tests[i]->run();
      }
}

void UnitTestSuite::intro() const
{
   if (osptr) {
      *osptr << "************************************************************************************************" << endl;
      *osptr << "* TMVA - S T R E S S and U N I T test suite                                                    *" << endl;
      *osptr << "************************************************************************************************" << endl;
   }
}

long UnitTestSuite::report() const
{
   if (osptr) {
      long totFail = 0;
      *osptr << "************************************************************************************************" << endl;
      *osptr << "* TMVA - U N I T test : Summary                                                                *" << endl;
      *osptr << "************************************************************************************************" << endl;
      size_t i;
      for (i = 0; i < tests.size(); ++i)
         {
            assert(tests[i]);
            *osptr << "Test " << setw(2) << i << " : ";
            totFail += tests[i]->report();
         }
      return totFail;
   }
   else
      return getNumFailed();
}

long UnitTestSuite::getNumPassed() const
{
   long totPass = 0;
   for (size_t i = 0; i < tests.size(); ++i)
      {
         assert(tests[i]);
         totPass += tests[i]->getNumPassed();
      }
   return totPass;
}

long UnitTestSuite::getNumFailed() const
{
   long totFail = 0;
   for (size_t i = 0; i < tests.size(); ++i)
      {
         assert(tests[i]);
         totFail += tests[i]->getNumFailed();
      }
   return totFail;
}

void UnitTestSuite::reset()
{
   for (size_t i = 0; i < tests.size(); ++i)
      {
         assert(tests[i]);
         tests[i]->reset();
      }
}
