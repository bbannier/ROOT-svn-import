//#include <Foundation/Foundation.h>

#include "CocoaUtils.h"

namespace ROOT {
namespace MacOSX {
namespace Util {

//______________________________________________________________________________
StrongReference::StrongReference()
                   : fNSObject(nil)
{
}

//______________________________________________________________________________
StrongReference::StrongReference(NSObject *nsObject)
                   : fNSObject([nsObject retain])
{
}

//______________________________________________________________________________
StrongReference::StrongReference(const StrongReference &rhs)
                   : fNSObject([rhs.fNSObject retain])
{
}

//______________________________________________________________________________
StrongReference::~StrongReference()
{
   [fNSObject release];
}

//______________________________________________________________________________
StrongReference &StrongReference::operator = (const StrongReference &rhs)
{
   if (&rhs != this) {
      //Even if both reference the same NSObject, it's ok to do release.
      [fNSObject release];
      fNSObject = [rhs.fNSObject retain];
   }
   
   return *this;
}

//______________________________________________________________________________
StrongReference &StrongReference::operator = (NSObject *nsObject)
{
   if (nsObject != fNSObject) {
      [fNSObject release];
      fNSObject = [nsObject retain];
   }
   
   return *this;
}

//______________________________________________________________________________
AutoreleasePool::AutoreleasePool()
                  : fPool([[NSAutoreleasePool alloc] init])
{
}

//______________________________________________________________________________
AutoreleasePool::~AutoreleasePool()
{
   [fPool release];
}

}
}
}
