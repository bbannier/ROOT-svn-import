#ifndef ROOT_CocoaPrivate
#define ROOT_CocoaPrivate

#include <vector>
#include <map>

#ifndef ROOT_CocoaUtils
#include "CocoaUtils.h"
#endif
#ifndef ROOT_X11Colors
#include "X11Colors.h"
#endif
#ifndef ROOT_GuiTypes
#include "GuiTypes.h"
#endif

@protocol X11Drawable;
@class NSWindow;

class TGCocoa;

namespace ROOT {
namespace MacOSX {
namespace Details {

class CocoaPrivate {
   friend class TGCocoa;
public:
   ~CocoaPrivate();
private:
   CocoaPrivate();
   
   CocoaPrivate(const CocoaPrivate &rhs) = delete;
   CocoaPrivate &operator = (const CocoaPrivate &rhs) = delete;

   void               InitX11RootWindow();
   unsigned           RegisterWindow(NSObject *nsWin);
   id<X11Drawable>    GetWindow(unsigned windowID)const;
   void               DeleteWindow(unsigned windowID);
   

   //Color "parser": either parse string like "#ddeeaa", or
   //search rgb.txt like table for named color.
   X11::ColorParser                          fX11ColorParser;
   //Id for the new registered window.
   unsigned                                  fCurrentWindowID;
   //Cache of ids.
   std::vector<unsigned>                     fFreeWindowIDs;
   //Cocoa objects (views, windows, "pixmaps").
   std::map<unsigned, Util::StrongReference> fWindows;
};

}
}
}

#endif
