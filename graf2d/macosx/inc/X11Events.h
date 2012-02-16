#ifndef ROOT_X11Events
#define ROOT_X11Events

#include <vector>

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

class EventTranslator {
public:
   EventTranslator();

   void GenerateCrossingEvent(QuartzView *viewUnderPointer, NSEvent *theEvent);
private:
   bool HasGrab()const;

   void GenerateCrossingEventNormal(QuartzView *view, NSEvent *theEvent);

   Ancestry FindRelation(QuartzView *view1, QuartzView *view2, QuartzView **lca);


   QuartzView *fViewUnderPointer;
   std::vector<QuartzView *> fBranch1;
   std::vector<QuartzView *> fBranch2;
};

}//X11
}//MacOSX
}//ROOT

#endif
