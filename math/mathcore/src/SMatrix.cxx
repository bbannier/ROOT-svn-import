#include "Math/MatrixRepresentationsStatic.h"

void ROOT::Math::RowOffsetsBase::init(int *v, int *offsets, unsigned int D)
{
   v[0]=0;
   for (unsigned int i=1; i<D; ++i)
      v[i]=v[i-1]+i;
      for (unsigned int i=0; i<D; ++i) {
         for (unsigned int j=0; j<=i; ++j)
            offsets[i*D+j] = v[i]+j;
         for (unsigned int j=i+1; j<D; ++j)
            offsets[i*D+j] = v[j]+i ;
  }
}
