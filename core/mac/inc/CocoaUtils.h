//Author: Timur Pocheptsov.
#ifndef ROOT_CocoaUtils
#define ROOT_CocoaUtils

#include <Foundation/Foundation.h>

namespace ROOT {
namespace MacOSX {
namespace Util {

//In principle, NS is a prefix for AppKit classes, but I do not want to make it suffix and
//still have to distinguish between RAII classes for AppKit and for Core Foundation/Core Graphics (suffix CF).
//But in C++ I have namespaces, in Obj-C they will never have (beleive me!), and I can have NSWhatIWant,
//since it will be ROOT::MacOSX::Util::NSWhatIWant. The same is true for CFWhatIWant.

/////////////////////////////////////////////////////////////////////
//                                                                 //
//  NSStrongReference. Class to keep strong reference to NSObject. //
//  Move ctor and assignment operator are deleted.                 //
//                                                                 //
/////////////////////////////////////////////////////////////////////

class NSStrongReference {
public:
   NSStrongReference();
   NSStrongReference(NSObject *nsObject);//increases reference counter.

   NSStrongReference(const NSStrongReference &rhs);//increases reference counter.
   //I declared this as deleted explicitly, not
   //to think about nice C++ rules :) The same for move assignment.
   NSStrongReference(NSStrongReference &&rhs) = delete;

   ~NSStrongReference();

   NSStrongReference &operator = (const NSStrongReference &rhs);
   NSStrongReference &operator = (NSObject *nsObject);
   //Declare as deleted for clarity.
   NSStrongReference &operator = (NSStrongReference &&rhs) = delete;
   
   NSObject *Get()const
   {
      return fNSObject;
   }
   
   void Reset(NSObject *object);//[fNSObject release]; fNSObject = [object retain]
private:
   NSObject *fNSObject;
};

///////////////////////////////////////////////////////////////////
//                                                               //
// NSScopeGuard. Copy/move operations are deleted.               //
//                                                               //
///////////////////////////////////////////////////////////////////

class NSScopeGuard {
public:
   explicit NSScopeGuard(NSObject *nsObject);
   ~NSScopeGuard();
   
   NSScopeGuard(const NSScopeGuard &rhs) = delete;
   //Declare as deleted for clarity.
   NSScopeGuard(NSScopeGuard &&rhs) = delete;
   
   NSScopeGuard &operator = (const NSScopeGuard &rhs) = delete;
   //Declare as deleted for clarity.
   NSScopeGuard &operator = (NSScopeGuard &&rhs) = delete;
   
   NSObject *Get()const
   {
      return fNSObject;
   }
   
   void Reset(NSObject *object);//[fNSObject relese]; fNSObject = object;
   void Release();
private:   
   NSObject *fNSObject;
};

//////////////////////////////////////
//                                  //
// RAII class for autorelease pool. //
//                                  //
//////////////////////////////////////

class AutoreleasePool {
public:
   AutoreleasePool();
   ~AutoreleasePool();
   

   AutoreleasePool(const AutoreleasePool &rhs) = delete;
   //Declare as deleted for clarity.
   AutoreleasePool(AutoreleasePool &&rhs) = delete;

   AutoreleasePool &operator = (const AutoreleasePool &rhs) = delete;
   //Declare as deleted for clarity.
   AutoreleasePool &operator = (AutoreleasePool &&rhs) = delete;
private:

   NSAutoreleasePool *fPool;
};

///////////////////////////////////////////////////////////
//                                                       //
// Strong reference for a Core Foundation object.        //
// This class can have specializations for CF object     //
// with it's own version of retain or release.           //
//                                                       //
///////////////////////////////////////////////////////////

template<class RefType>
class CFStrongReference {
public:
   CFStrongReference()
              : fRef(nullptr)
   {
   }
   
   CFStrongReference(RefType ref, bool initRetain)
              : fRef(ref)
   {
      if (initRetain && ref)
         CFRetain(ref);
   }
   
   CFStrongReference(const CFStrongReference &rhs)
   {
      fRef = rhs.fRef;
      if (fRef)
         CFRetain(fRef);
   }

   CFStrongReference &operator = (const CFStrongReference &rhs)
   {
      if (this != &rhs) {
         if (fRef)
            CFRelease(fRef);//Ok even if rhs references the same.
         fRef = rhs.fRef;
         if (fRef)
            CFRetain(fRef);
      }
      
      return *this;
   }
   
   ~CFStrongReference()
   {
      if (fRef)
         CFRelease(fRef);
   }
   
   RefType Get()const
   {
      return fRef;
   }
   
   //Declare as deleted for clarity.
   CFStrongReference(CFStrongReference &&rhs) = delete;
   CFStrongReference &operator = (CFStrongReference &&rhs) = delete;
   
private:
   RefType fRef;
};

///////////////////////////////////////////////////
//                                               //
// Scope guard for Core Foundations objects.     //
// Specializations can be defined to call        //
// something different from CFRetain/CFRelease,  //
// but no need, they usually differ by accepting //
// null pointer (CFRetain/CFRelease will cause   //
// an error.                                     //
//                                               //
///////////////////////////////////////////////////

template<class RefType>
class CFScopeGuard {
public:
   CFScopeGuard()
            : fRef(nullptr)
   {
   }
   
   explicit CFScopeGuard(RefType ref)
               : fRef(ref)
   {
   }
   
   ~CFScopeGuard()
   {
      if (fRef)
         CFRelease(fRef);
   }
   
   CFScopeGuard(const CFScopeGuard &rhs) = delete;
   CFScopeGuard(CFScopeGuard &&rhs) = delete;
   
   //Declare as delete for clarity.
   CFScopeGuard &operator = (const CFScopeGuard &rhs) = delete;
   CFScopeGuard &operator = (CFScopeGuard &&rhs) = delete;
   
   RefType Get()const
   {
      return fRef;
   }
   
   void Reset(RefType ref)
   {
      if (ref != fRef) {
         if (fRef)
            CFRelease(fRef);
         fRef = ref;
      }
   }
   
   void Release()
   {
      fRef = nullptr;
   }

private:
   RefType fRef;
};

}
}
}

#endif
