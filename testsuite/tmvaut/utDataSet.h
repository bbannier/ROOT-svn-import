#ifndef UTDATASET_H
#define UTDATASET_H

// Author: Christoph Rosemann   Dec. 2009
// TMVA unit tests

#include "UnitTest.h"

#include <vector>

namespace TMVA {
   class DataSet;
   class DataSetInfo;
   class Event;
}

class utDataSet : public UnitTesting::UnitTest
{
public:
   utDataSet();
   void run();

private:
   void testMethods();

   TMVA::DataSet*            dataset;
   TMVA::DataSetInfo*        datasetinfo;
   TMVA::Event*              event0;
   TMVA::Event*              event1;
   TMVA::Event*              event2;
   TMVA::Event*              event3;
   TMVA::Event*              event4;
   std::vector<TMVA::Event>* vecevent;
   //  TMVA::Results*            result;
};
#endif // UTDATASET_H
