#ifndef ROOT_CocoaPrivate
#define ROOT_CocoaPrivate

#include <vector>

#ifndef ROOT_CocoaUtils
#include "CocoaUtils.h"
#endif
#ifndef ROOT_X11Colors
#include "X11Colors.h"
#endif
#ifndef ROOT_GuiTypes
#include "GuiTypes.h"
#endif

@class NSWindow;

class TGCocoa;

namespace ROOT {
namespace MacOSX {
namespace Details {

struct CocoaWindowAttributes {
   WindowAttributes_t fROOTWindowAttribs;
   Util::StrongReference fCocoaWindow;
   
   CocoaWindowAttributes();
};

class CocoaPrivate {
   friend class TGCocoa;
public:
   ~CocoaPrivate();
private:
   CocoaPrivate();
   
   CocoaPrivate(const CocoaPrivate &rhs) = delete;
   CocoaPrivate &operator = (const CocoaPrivate &rhs) = delete;

   void InitX11RootWindow();
   
   X11::ColorParser fX11ColorParser;
   std::vector<CocoaWindowAttributes> fWindows;
};

}
}
}

#endif
