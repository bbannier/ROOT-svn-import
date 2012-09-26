#include "RooStats/CombinedDataSet.h"

template class CombinedDataSet<Double_t>;

using namespace RooStats;

CombinedDataSet::CombinedDataSet(std::vector<RooDataSet*> dataSets) :
   fNumberOfDataSets(dataSets.size()),
   fDataSets(dataSets)
{
}


