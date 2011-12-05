#ifndef ROOT_TMacOSXSystem
#define ROOT_TMacOSXSystem

#include <memory>

#ifndef ROOT_TUnixSystem
#include "TUnixSystem.h"
#endif

//
// Event loop with MacOS X and Cocoa is different
// from TUnixSystem. At the moment, I see this difference
// primarily in DispatchOneEvent member-function. So I 
// simply inherit TUnixSystem to just override DispatchOneEvent.
// Class is placed in graf2d/macosx only in development branch,
// later it should go into core/macosx, and graf2d/macosx module can
// be renamed to 'graf2d/cocoa' or 'graf2d/quartz'.
//

//'Private' pimpl to hide from CINT Objective-C specific (if I have one).
class TMacOSXSystemPrivate;

class TMacOSXSystem : public TUnixSystem {
public:
   TMacOSXSystem();
   ~TMacOSXSystem();
   
   void DispatchOneEvent(Bool_t pendingOnly);

private:

   std::auto_ptr<TMacOSXSystemPrivate> fPimpl; //!

   TMacOSXSystem(const TMacOSXSystem &rhs);
   TMacOSXSystem &operator = (const TMacOSXSystem &rhs);
   
   ClassDef(TMacOSXSystem, 0);//TSystem's implementation for Mac OSX.
};

#endif
