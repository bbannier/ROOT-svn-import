#ifndef ROOT_CocoaUtils
#define ROOT_CocoaUtils

@class NSAutoreleasePool;
@class NSObject;


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
   
   NSObject *Get()const;
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

}
}
}

#endif
