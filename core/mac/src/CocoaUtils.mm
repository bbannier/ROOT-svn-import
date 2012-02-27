#include "CocoaUtils.h"

namespace ROOT {
namespace MacOSX {
namespace Util {

//______________________________________________________________________________
StrongReferenceNS::StrongReferenceNS()
                   : fNSObject(nil)
{
}

//______________________________________________________________________________
StrongReferenceNS::StrongReferenceNS(NSObject *nsObject)
                   : fNSObject([nsObject retain])
{
}

//______________________________________________________________________________
StrongReferenceNS::StrongReferenceNS(const StrongReferenceNS &rhs)
                   : fNSObject([rhs.fNSObject retain])
{
}

//______________________________________________________________________________
StrongReferenceNS::~StrongReferenceNS()
{
   [fNSObject release];
}

//______________________________________________________________________________
StrongReferenceNS &StrongReferenceNS::operator = (const StrongReferenceNS &rhs)
{
   if (&rhs != this) {
      //Even if both reference the same NSObject, it's ok to do release.
      [fNSObject release];
      fNSObject = [rhs.fNSObject retain];
   }
   
   return *this;
}

//______________________________________________________________________________
StrongReferenceNS &StrongReferenceNS::operator = (NSObject *nsObject)
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
