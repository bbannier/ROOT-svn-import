// extra include file for Vc
// has to be added at beginning

#ifndef VCSMATRIX_0
#define VCSMATRIX_0

#include "Vc/Vc"

//using Vc::abs;

namespace std {
   using Vc::abs;
   using Vc::sqrt;
   using Vc::min;
   using Vc::max;
}

// for operator << of Vc
namespace ROOT {
namespace Math {
   std::ostream & operator  << (std::ostream &out, const Vc::double_v  &v) {
      return ::operator<<(out, v);
   }
}
}




#endif
