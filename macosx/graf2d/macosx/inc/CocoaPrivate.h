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

@protocol RootGUIElement;
@class NSWindow;

class TGCocoa;

namespace ROOT {
namespace MacOSX {
namespace Details {

struct CocoaWindowAttributes {
   WindowAttributes_t fROOTWindowAttribs;
   Util::StrongReference fCocoaWindow;
   
   CocoaWindowAttributes();
   CocoaWindowAttributes(const WindowAttributes_t &winAttr, NSObject *nsWin);
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

   unsigned RegisterWindow(NSObject *nsWin, const WindowAttributes_t &winAttr);
   id<RootGUIElement> GetWindow(unsigned windowID)const;
   const WindowAttributes_t &GetWindowAttributes(unsigned winID)const;
   
   void DeleteWindow(unsigned windowID);
   
   X11::ColorParser fX11ColorParser;

   unsigned fCurrentWindowID;
   std::vector<unsigned> fFreeWindowIDs;
   std::map<unsigned, CocoaWindowAttributes> fWindows;
   
   NSEvent *fPendingEvent;
};

}
}
}

#endif
