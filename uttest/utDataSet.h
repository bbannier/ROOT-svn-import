#ifndef UTDATASET_H
#define UTDATASET_H

// Author: Christoph Rosemann   Dec. 2009
// TMVA unit tests

#include <vector>

#include "TTree.h"

#include "DataSet.h"
#include "DataSetInfo.h"
#include "Event.h"
#include "Types.h"
#include "Results.h"
#include "UnitTest.h"

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
