#ifndef ROOT_CocoaUtils
#define ROOT_CocoaUtils

#include <Foundation/Foundation.h>

namespace ROOT {
namespace MacOSX {
namespace Util {

class StrongReference {
public:
   StrongReference();
   StrongReference(NSObject *nsObject);
   //Check also, if I want &&.
   StrongReference(const StrongReference &rhs);

   
   ~StrongReference();

   StrongReference &operator = (const StrongReference &rhs);
   StrongReference &operator = (NSObject *nsObject);
   
   NSObject *Get()const
   {
      return fNSObject;
   }
private:
   NSObject *fNSObject;
};


//Replace with more generic RAII class later.
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

//Replace with more generic RAII class later.
template<class RefType>
class CFGuard {
public:
   CFGuard()
      : fFdRef()
   {
   }
   
   explicit CFGuard(RefType ref, bool initRetain)
               : fFdRef(ref)
   {
      if (initRetain)
         CFRetain(ref);
   }
   
   CFGuard(const CFGuard &rhs)
   {
      fFdRef = rhs.fFdRef;
      CFRetain(fFdRef);
   }
   
   //TODO: Check ravlue references also.
   CFGuard &operator = (const CFGuard &rhs)
   {
      if (this != &rhs) {
         CFRelease(fFdRef);
         fFdRef = rhs.fFdRef;
         CFRetain(fFdRef);
      }
      
      return *this;
   }

   ~CFGuard()
   {
      CFRelease(fFdRef);
   }
   
   RefType Get()const
   {
      return fFdRef;
   }
   
private:
   RefType fFdRef;
};

}
}
}

#endif
