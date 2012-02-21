#ifndef ROOT_X11Events
#define ROOT_X11Events

#include <vector>

#ifndef ROOT_GuiTypes
#include "GuiTypes.h"
#endif

//////////////////////////////////////////////////////////////////////////
//                                                                      //
// EventTranslator class translates Cocoa events to 'ROOT's X11' events.//
// In 90% cases there is no direct mapping from Cocoa event to          //
// X11 event: Cocoa events are more simple (from user's POV).           //
// EventTranslator tries to emulate X11 behavior.                       //
//                                                                      //
//////////////////////////////////////////////////////////////////////////

@class QuartzView;
@class NSEvent;

namespace ROOT {
namespace MacOSX {
namespace X11 {//X11 emulation for Cocoa.

enum class Ancestry {
   view1IsParent,
   view2IsParent,
   haveNonRootAncestor,
   ancestorIsRoot
};

enum class PointerGrab {
   noGrab,
   implicitGrab,
   activeGrab,
   passiveGrab//?
};

class EventTranslator {

public:
   EventTranslator();

   void GenerateCrossingEvent(QuartzView *viewUnderPointer, NSEvent *theEvent);
   void GeneratePointerMotionEvent(QuartzView *eventView, NSEvent *theEvent);
   void GenerateButtonPressEvent(QuartzView *eventView, NSEvent *theEvent, EMouseButton btn);
   void GenerateButtonReleaseEvent(QuartzView *eventView, NSEvent *theEvent, EMouseButton btn);

private:
   bool HasPointerGrab()const;

   void GenerateCrossingEvent(QuartzView *viewUnderPointer, NSEvent *theEvent, EXMagic detail);
   void GeneratePointerMotionEventNoGrab(QuartzView *view, NSEvent *theEvent);

   void GenerateButtonPressEventNoGrab(QuartzView *view, NSEvent *theEvent, EMouseButton btn);

   void GenerateButtonReleaseEventImplicitGrab(NSEvent *theEvent, EMouseButton btn);
   void GenerateButtonReleaseEventActiveGrab(QuartzView *eventView, NSEvent *theEvent, EMouseButton btn);

   Ancestry FindRelation(QuartzView *view1, QuartzView *view2, QuartzView **lca);

   QuartzView *fViewUnderPointer;
   std::vector<QuartzView *> fBranch1;
   std::vector<QuartzView *> fBranch2;
   
   PointerGrab fPointerGrab;
//   EMouseButton fButtonPressed;//for implicit grab.
   QuartzView *fCurrentGrabView;
};

}//X11
}//MacOSX
}//ROOT

#endif
