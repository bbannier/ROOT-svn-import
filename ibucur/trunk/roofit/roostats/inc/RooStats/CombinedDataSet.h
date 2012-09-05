#ifndef ROOSTATS_CombinedDataSet
#define ROOSTATS_CombinedDataSet

#include "RooDataSet.h"

namespace RooStats {
   class CombinedDataSet : public RooDataSet, public AbstractDataSet {
   public:
      virtual ~CombinedDataSet() {}
   private:
      CombinedDataSet() {}
      ClassDef(CombinedDataSet, 1)
   };
}

#endif // ROOSTATS_CombinedDataSet


