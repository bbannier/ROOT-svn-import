#include "utVariableInfo.h"
#include "TMath.h"

#include "TMVA/VariableInfo.h"

using namespace std;
using namespace UnitTesting;
using namespace TMVA;

utVariableInfo::utVariableInfo() :
   UnitTest("VariableInfo", __FILE__)
{
   expression = "expression";
   title      = "title";
   unit       = "unit";

   varCounter = 123;
   varType    = 'D';
   min        = 2.781828;
   max        = 3.1416;
   normalized = kTRUE;
   external   = &max;

   mean       = 42.;
   rms        = 47.11;
}



void utVariableInfo::run()
{
   _testConstructor1();
   _testConstructor2();
   _testConstructor3();
   _testMethods();
}



void utVariableInfo::_testConstructor1()
{
   _varinfoC1 = new VariableInfo( expression, title,  unit, varCounter, varType, external, min, max, normalized);

   test_(_varinfoC1->GetExpression()  == expression);
   //  test_(_varinfoC1->GetInternalName() == );
   //  test_(_varinfoC1->GetLabel()
   test_(_varinfoC1->GetTitle()   == title);
   test_(_varinfoC1->GetUnit()    == unit);
   test_(_varinfoC1->GetVarType() == varType);

   test_(_varinfoC1->GetMin()  == min);
   test_(_varinfoC1->GetMax()  == max);
   //   test_(_varinfoC1->GetMean() == 0.);
   //   test_(_varinfoC1->GetRMS()  == 0.);
   test_(_varinfoC1->GetExternalLink() == external);

}



void utVariableInfo::_testConstructor2()
{
   _varinfoC2 = new VariableInfo();
   test_(_varinfoC2->GetExpression()  == "");
   //  test_(_varinfoC2->GetInternalName() == );
   //  test_(_varinfoC2->GetLabel()
   test_(_varinfoC2->GetTitle()   == "");
   test_(_varinfoC2->GetUnit()    == "");
   test_(_varinfoC2->GetVarType() == '\0');

   test_(_varinfoC2->GetMin()  == 1e30);
   test_(_varinfoC2->GetMax()  == -1e30);
   //   test_(_varinfoC2->GetMean() == 0.);
   //   test_(_varinfoC2->GetRMS()  == 0.);
   //  test_(_varinfoC2->GetExternalLink() == external);
}



void utVariableInfo::_testConstructor3()
{
   _varinfoC3 = new VariableInfo(*_varinfoC1);

   test_(_varinfoC3->GetExpression()  == expression);
   //  test_(_varinfoC3->GetInternalName() == );
   //  test_(_varinfoC3->GetLabel()
   test_(_varinfoC3->GetTitle()   == title);
   test_(_varinfoC3->GetUnit()    == unit);
   test_(_varinfoC3->GetVarType() == varType);

   test_(_varinfoC3->GetMin()  == min);
   test_(_varinfoC3->GetMax()  == max);
   //   test_(_varinfoC3->GetMean() == 0.);
   //   test_(_varinfoC3->GetRMS()  == 0.);
   test_(_varinfoC3->GetExternalLink() == external);
}



void utVariableInfo::_testMethods()
{
   _varinfoC2->SetMin(min);
   _varinfoC2->SetMax(max);
   _varinfoC2->SetMean(mean);
   _varinfoC2->SetRMS(rms);
   _varinfoC2->SetExternalLink(external);

   test_(_varinfoC2->GetMin()  == min);
   test_(_varinfoC2->GetMax()  == max);
   test_(_varinfoC2->GetMean() == mean);
   test_(_varinfoC2->GetRMS()  == rms);

   _varinfoC2->ResetMinMax();
   test_(_varinfoC2->GetMin()  == 1e30);
   test_(_varinfoC2->GetMax()  == -1e30);

   // test assignment
   *_varinfoC2 = *_varinfoC1;

   test_(_varinfoC2->GetExpression()  == expression);
   //  test_(_varinfoC2->GetTitle()   == title); // title is not copied
   //  test_(_varinfoC2->GetUnit()    == unit);  // unit is not copied
   test_(_varinfoC2->GetVarType() == varType);

   test_(_varinfoC2->GetMin()  == min);
   test_(_varinfoC2->GetMax()  == max);
   test_(_varinfoC2->GetExternalLink() == external);
}




