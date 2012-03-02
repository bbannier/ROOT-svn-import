//Author: Timur Pocheptsov.
#ifndef ROOT_CocoaUtils
#define ROOT_CocoaUtils

#include <Foundation/Foundation.h>

namespace ROOT {
namespace MacOSX {
namespace Util {

//NS suffix == Cocoa classes. Not NSStrongReference, since
//it will look like it's a Cocoa class.
//Strong reference for Core Foundation objects (== CF).
//Again, I'm using suffix, since with prefix it
//looks like Core Foundation type.
//These classes must be replaced by generic solution, which works with Cocoa and Core Foundation
//resources + release function can be passed as a template parameter.
//Such classes can be also used by my ios module for Core Foundation objects (Cocoa objects on
//ios are managed by ARC, no need in RAII in that case). I'll try to add this nice features next week.

class StrongReferenceNS {
public:
   StrongReferenceNS();
   StrongReferenceNS(NSObject *nsObject);
   //Check &&.
   StrongReferenceNS(const StrongReferenceNS &rhs);

   
   ~StrongReferenceNS();

   StrongReferenceNS &operator = (const StrongReferenceNS &rhs);
   StrongReferenceNS &operator = (NSObject *nsObject);
   
   NSObject *Get()const
   {
      return fNSObject;
   }
   
   void Reset(NSObject *object);
private:
   NSObject *fNSObject;
};

//RAII class for autorelease pool.
class AutoreleasePool {
public:
   AutoreleasePool();
   ~AutoreleasePool();
   
   //Check &&
   AutoreleasePool(const AutoreleasePool &rhs) = delete;
   AutoreleasePool &operator = (const AutoreleasePool &rhs) = delete;
private:
   NSAutoreleasePool *fPool;
};

template<class RefType>
class StrongReferenceCF {
public:
   StrongReferenceCF()
      : fRef(0)
   {
   }
   
   explicit StrongReferenceCF(RefType ref, bool initRetain)
               : fRef(ref)
   {
      if (initRetain && ref)
         CFRetain(ref);
   }
   
   StrongReferenceCF(const StrongReferenceCF &rhs)
   {
      fRef = rhs.fRef;
      if (fRef)
         CFRetain(fRef);
   }
   
   //TODO: &&
   StrongReferenceCF &operator = (const StrongReferenceCF &rhs)
   {
      if (this != &rhs) {
         if (fRef)
            CFRelease(fRef);
         fRef = rhs.fRef;
         if (fRef)
            CFRetain(fRef);
      }
      
      return *this;
   }

   ~StrongReferenceCF()
   {
      if (fRef)
         CFRelease(fRef);
   }
   
   RefType Get()const
   {
      return fRef;
   }
   
private:
   RefType fRef;
};

}
}
}

#endif
