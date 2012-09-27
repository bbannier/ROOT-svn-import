#include "RooStats/CombinedDataSet.h"

using namespace RooStats;

ClassImp(CombinedDataSet<Double_t>);

template<typename T>
CombinedDataSet<T>::CombinedDataSet(std::vector<RooDataSet*> dataSets) :
   fNumberOfDataSets(dataSets.size()),
   fDataSets(dataSets)
{
}

template class CombinedDataSet<Double_t>;

