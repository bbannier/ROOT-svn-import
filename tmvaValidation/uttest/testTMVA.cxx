
// Author: Christoph Rosemann   Dec. 2009
// TMVA unit tests

#include <iostream>
#include "UnitTestSuite.h"
#include "utEvent.h"
#include "utVariableInfo.h"
#include "utDataSetInfo.h"
#include "utDataSet.h"
using namespace UnitTesting;
using namespace std;

int main() 
{
  UnitTestSuite TMVA_test("TMVA unit testing");
  
  TMVA_test.addTest(new utEvent);
  TMVA_test.addTest(new utVariableInfo);
  TMVA_test.addTest(new utDataSetInfo);
  TMVA_test.addTest(new utDataSet);
  TMVA_test.run();
  
  long int nFail = TMVA_test.report();
  cout << " SUMMARY: " << endl;
  cout << " Total number of failures: " << nFail << endl;
  //  return eventTest.report();
}
