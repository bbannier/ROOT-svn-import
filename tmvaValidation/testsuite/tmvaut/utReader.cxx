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
   Reader* reader = new Reader();
   delete reader;

   float xtest;
   Reader* reader2 = new Reader();
   Reader* reader3 = new Reader();
   reader2->AddVariable("test", &xtest);
   reader3->AddVariable("test", &xtest);
   delete reader2;
   delete reader3;
   test_(1>0); 

}
