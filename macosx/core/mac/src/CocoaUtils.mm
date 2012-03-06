//Author: Timur Pocheptsov.
#include "CocoaUtils.h"

namespace ROOT {
namespace MacOSX {
namespace Util {

//______________________________________________________________________________
NSStrongReference::NSStrongReference()
                   : fNSObject(nil)
{
}

//______________________________________________________________________________
NSStrongReference::NSStrongReference(NSObject *nsObject)
                   : fNSObject([nsObject retain])
{
}

//______________________________________________________________________________
NSStrongReference::NSStrongReference(const NSStrongReference &rhs)
                   : fNSObject([rhs.fNSObject retain])
{
}

//______________________________________________________________________________
NSStrongReference::~NSStrongReference()
{
   [fNSObject release];
}

//______________________________________________________________________________
NSStrongReference &NSStrongReference::operator = (const NSStrongReference &rhs)
{
   if (&rhs != this) {
      //Even if both reference the same NSObject, it's ok to do release.
      [fNSObject release];
      fNSObject = [rhs.fNSObject retain];
   }
   
   return *this;
}

//______________________________________________________________________________
NSStrongReference &NSStrongReference::operator = (NSObject *nsObject)
{
   if (nsObject != fNSObject) {
      [fNSObject release];
      fNSObject = [nsObject retain];
   }
   
   return *this;
}

//______________________________________________________________________________
void NSStrongReference::Reset(NSObject *object)
{
   if (fNSObject != object) {
      NSObject *obj = [object retain];//Haha, is it possible to have 2 different pointers on the same object in Obj-C? :)
      [fNSObject release];
      fNSObject = obj;
   }
}

//______________________________________________________________________________
NSScopeGuard::NSScopeGuard(NSObject *nsObject)
          : fNSObject(nsObject)
{
}

//______________________________________________________________________________
NSScopeGuard::~NSScopeGuard()
{
   [fNSObject release];//nothing for nil.
}
   
//______________________________________________________________________________
void NSScopeGuard::Reset(NSObject *object)
{
   if (object != fNSObject) {
      [fNSObject release];
      fNSObject = object;
   }
}

//______________________________________________________________________________
void NSScopeGuard::Release()
{
   fNSObject = nil;
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
