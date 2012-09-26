#ifndef ROOSTATS_CombinedDataSet
#define ROOSTATS_CombinedDataSet

#include "RooStats/AbstractDataSet.h"
#include "RooDataSet.h"
#include <vector>

namespace RooStats {
   template<typename T>
   class CombinedDataSet : public TObject, public AbstractDataSet {
   public:
      virtual ~CombinedDataSet() {}
      CombinedDataSet(std::vector<RooDataSet*> dataSets);
      

   private:
      CombinedDataSet() {} // hide default constructor (needed for I/O)
      CombinedDataSet(const CombinedDataSet& rhs); // disallow copy constructor
      CombinedDataSet& operator=(const CombinedDataSet& rhs); // disallow assignment operator

      Int_t fNumberOfDataSets;
      std::vector<RooDataSet *> fDataSets;
      std::vector<T> fRawData;
      std::vector<DataSetInfo> fDetails;

      struct DataSetInfo {
         Int_t firstIdx;
         Int_t lastIdx;
         Int_t numRows;
         Int_t numColumns;
         Bool_t rowMajor;
      }


      ClassDef(CombinedDataSet, 1)
   };
}

#endif // ROOSTATS_CombinedDataSet


