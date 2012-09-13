#include "RooStats/CombinedDataSet.h"

using namespace RooStats;

CombinedDataSet::CombinedDataSet(std::vector<RooDataSet*> dataSets) :
   fNumberOfDataSets(dataSets.size()),
   fDataSets(dataSets)
{
}


