
// Author: Christoph Rosemann   Dec. 2009
// TMVA unit tests

#include <iostream>
#include "tmvaut/UnitTestSuite.h"
#include "tmvaut/utEvent.h"
#include "tmvaut/utVariableInfo.h"
#include "tmvaut/utDataSetInfo.h"
#include "tmvaut/utDataSet.h"
using namespace UnitTesting;
using namespace std;

int main()
{
   UnitTestSuite TMVA_test("TMVA unit testing");

   TMVA_test.intro();

   TMVA_test.addTest(new utEvent);
   TMVA_test.addTest(new utVariableInfo);
   TMVA_test.addTest(new utDataSetInfo);
   TMVA_test.addTest(new utDataSet);
   TMVA_test.run();

   long int nFail = TMVA_test.report();
   cout << "Total number of failures: " << nFail << endl;
   cout << "************************************************************************************************" << endl;
   //  return eventTest.report();
}
