#ifndef ROOT_CocoaUtils
#define ROOT_CocoaUtils

#include <Foundation/Foundation.h>

namespace ROOT {
namespace MacOSX {
namespace Util {

//NS suffix == Cocoa classes. Not NSStrongReference, since
//it will look like it's a Cocoa class.
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

//Strong reference for Core Foundation objects (== CF).
//Again, I'm using suffix, since with prefix it
//looks like Core Foundation type.
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
