// @(#)root/graf2d:$Id$
// Author: Timur Pocheptsov, 11/06/2012

/*************************************************************************
 * Copyright (C) 1995-2011, Rene Brun and Fons Rademakers.               *
 * All rights reserved.                                                  *
 *                                                                       *
 * For the licensing terms see $ROOTSYS/LICENSE.                         *
 * For the list of contributors see $ROOTSYS/README/CREDITS.             *
 *************************************************************************/

#ifndef ROOT_QuartzUtils
#define ROOT_QuartzUtils

#include <ApplicationServices/ApplicationServices.h>

#ifndef ROOT_Rtypes
#include "Rtypes.h"
#endif

namespace ROOT {
namespace Quartz {

//Scope guard class for CGContextRef.
class CGStateGuard {
public:
   CGStateGuard(CGContextRef ctx);
   ~CGStateGuard();
   
private:
   CGContextRef fCtx;
   
   CGStateGuard(const CGStateGuard &rhs);
   CGStateGuard &operator = (const CGStateGuard &rhs);
};

}
}

#endif
