#include "utReader.h"

#include <string>
#include <iostream>
#include <cassert>
#include <vector>

#include "TTree.h"
#include "TString.h"

#include "TMVA/Reader.h"
#include "TMVA/Types.h"

#include "UnitTest.h"

using namespace std;
using namespace UnitTesting;
using namespace TMVA;

utReader::utReader(const char* /*theOption*/) 
   : UnitTest(string("Reader"))
{ 

}
utReader::~utReader(){ }

void utReader::run()
{
   float xtest,xtest2;
   Reader* reader2 = new Reader();
   Reader* reader3 = new Reader();
   reader2->AddVariable("test", &xtest);
   reader2->AddVariable("test2", &xtest2);
   reader3->AddVariable("test", &xtest);
   
   delete reader2;
   delete reader3;
   test_(1>0); 
   const int nTest=3;
   int ievt;
   vector<float> testvar(10);
   std::vector< TMVA::Reader* > reader(nTest);
   for (int iTest=0;iTest<nTest;iTest++){
#ifdef VERBOSE
      reader[iTest] = new TMVA::Reader( "!Color:!Silent" );
#else
      reader[iTest] = new TMVA::Reader( "!Color:Silent" );
#endif
      if (iTest==0){
         reader[iTest]->AddVariable( "var0" ,&testvar[0]);
         reader[iTest]->AddVariable( "var1" ,&testvar[1]);
         reader[iTest]->AddSpectator( "ievt" ,&ievt);
         reader[iTest]->BookMVA( "LD method", "weights/TMVATest_LD.weights.xml") ;
      }
      if (iTest==1){
         reader[iTest]->AddVariable( "var0" ,&testvar[0]);
         reader[iTest]->AddVariable( "var1" ,&testvar[1]);
         reader[iTest]->AddVariable( "var2" ,&testvar[2]);
         reader[iTest]->AddSpectator( "ievt" ,&ievt);
         reader[iTest]->BookMVA( "LD method", "weights/TMVATest3Var_LD.weights.xml") ;
      }
      if (iTest==2){
         reader[iTest]->AddVariable( "var0" ,&testvar[0]);
         reader[iTest]->AddVariable( "var1" ,&testvar[1]);
         reader[iTest]->AddVariable( "var2" ,&testvar[2]);
         reader[iTest]->AddVariable( "ivar0" ,&testvar[3]);
         reader[iTest]->AddVariable( "ivar1" ,&testvar[4]);
         reader[iTest]->AddSpectator( "ievt" ,&ievt);
         reader[iTest]->BookMVA( "LD method", "weights/TMVATest3VarF2VarI_LD.weights.xml") ;
      }
   }
   reader[0]->EvaluateMVA( "LD method");
   reader[1]->EvaluateMVA( "LD method");
   reader[2]->EvaluateMVA( "LD method");
   test_(1>0);
}
