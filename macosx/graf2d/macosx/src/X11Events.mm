#include <algorithm>
#include <utility>
#include <cassert>

#include <Cocoa/Cocoa.h>

#include "QuartzWindow.h"
#include "CocoaUtils.h"
#include "X11Events.h"
#include "TGClient.h"
#include "TGWindow.h"

namespace ROOT {
namespace MacOSX {
namespace X11 {

namespace Detail {

//Several aux. functions to extract parameters from Cocoa events.

//______________________________________________________________________________
Time_t TimeForCocoaEvent(NSEvent *theEvent)
{
   //1. Event is not nil.
   assert(theEvent != nil && "TimeForCocoaEvent, event parameter is nil");

   return [theEvent timestamp] * 1000;//TODO: check this!
}

//______________________________________________________________________________
Event_t NewX11EventFromCocoaEvent(unsigned windowID, NSEvent *theEvent)
{
   //1. Event is not nil.

   assert(theEvent != nil && "NewX11EventFromCocoaEvent, event parameter is nil");

   Event_t newEvent = {};
   newEvent.fWindow = windowID;
   newEvent.fTime = TimeForCocoaEvent(theEvent);
   return newEvent;
}

//______________________________________________________________________________
void ConvertEventLocationToROOTXY(NSEvent *cocoaEvent, QuartzView *eventView, Event_t *rootEvent)
{
   //1. All parameters are valid.
   //Both event and view must be in the same window, I do not check this here.

   assert(cocoaEvent != nil && "ConvertEventLocationToROOTXY, cocoaEvent parameter is nil");
   assert(eventView != nil && "ConvertEventLocationToROOTXY, eventView parameter is nil");
   assert(rootEvent != nullptr && "ConvertEventLocationToROOTXY, rootEvent parameter is null");

   //TODO: can [event window] be nil? (this can probably happen with mouse grabs).
   if (![cocoaEvent window])
      NSLog(@"Error in ConvertEventLocationToROOTXY, window property of event is nil, can not convert coordinates correctly");
   
   const NSPoint screenPoint = [[cocoaEvent window] convertBaseToScreen : [cocoaEvent locationInWindow]];
   NSPoint viewPoint = [[eventView window] convertScreenToBase : screenPoint];
   viewPoint = [eventView convertPointFromBase : viewPoint];

   rootEvent->fX = viewPoint.x;
   rootEvent->fY = ROOT::MacOSX::X11::LocalYCocoaToROOT(eventView, viewPoint.y);

   WindowAttributes_t attr = {};
   GetRootWindowAttributes(&attr);

   rootEvent->fXRoot = screenPoint.x;
   rootEvent->fYRoot = attr.fHeight - screenPoint.y;
}

//______________________________________________________________________________
unsigned GetKeyboardModifiersFromCocoaEvent(NSEvent *theEvent)
{
   assert(theEvent != nil && "GetKeyboardModifiersFromCocoaEvent, event parameter is nil");

   const NSUInteger modifiers = [theEvent modifierFlags];
   unsigned rootModifiers = 0;
   if (modifiers & NSAlphaShiftKeyMask)
      rootModifiers |= kKeyLockMask;
   if (modifiers & NSShiftKeyMask)
      rootModifiers |= kKeyShiftMask;
   if (modifiers & NSControlKeyMask)
      rootModifiers |= kKeyControlMask;
   if (modifiers & NSAlternateKeyMask)
      rootModifiers |= kKeyMod1Mask;
   if (modifiers & NSCommandKeyMask)
      rootModifiers |= kKeyMod2Mask;

   return rootModifiers;
}

//______________________________________________________________________________
unsigned GetModifiersFromCocoaEvent(NSEvent *theEvent)
{
   assert(theEvent != nil && "GetModifiersFromCocoaEvent, event parameter is nil");

   unsigned rootModifiers = GetKeyboardModifiersFromCocoaEvent(theEvent);
   const NSUInteger buttons = [NSEvent pressedMouseButtons];
   if (buttons & 1)
      rootModifiers |= kButton1Mask;
   if (buttons & 2)
      rootModifiers |= kButton2Mask;

   return rootModifiers;
}

//Misc. aux. functions.

//______________________________________________________________________________
bool IsParent(QuartzView *testParent, QuartzView *testChild)
{
   assert(testParent != nil && "IsParent, testParent parameter is nil");
   assert(testChild != nil && "IsParent, testChild parameter is nil");

   if (testChild.fParentView) {
      QuartzView *parent = testChild.fParentView;
      while (parent) {
         if(parent == testParent)
            return true;
         parent = parent.fParentView;
      }
   }

   return false;
}

//______________________________________________________________________________
void BuildAncestryBranch(QuartzView *view, std::vector<QuartzView *> &branch)
{
   assert(view != nil && "BuildAncestryBranch, view parameter is nil");
   assert(view.fParentView != nil && "BuildAncestryBranch, view must have a parent");
   assert(view.fLevel > 0 && "BuildAncestryBranch, view has nested level 0");

   branch.resize(view.fLevel);
   
   QuartzView *parent = view.fParentView;
   for (auto iter = branch.rbegin(), endIter = branch.rend(); iter != endIter; ++iter) {
      assert(parent != nil && "BuildAncestryBranch, fParentView is nil");
      *iter = parent;
      parent = parent.fParentView;
   }
}

//______________________________________________________________________________
Ancestry FindLowestCommonAncestor(QuartzView *view1, std::vector<QuartzView *> &branch1, 
                                  QuartzView *view2, std::vector<QuartzView *> &branch2, 
                                  QuartzView **lca)
{
   //Search for the lowest common ancestor.
   //View1 can not be parent of view2, view2 can not be parent of view1,
   //I do not check this condition here.

   assert(view1 != nil && "FindLowestCommonAncestor, view1 parameter is nil");
   assert(view2 != nil && "findLowestCommonAncestor, view2 parameter is nil");
   assert(lca != nullptr && "FindLowestCommonAncestor, lca parameter is null");
   
   if (!view1.fParentView)
      return Ancestry::ancestorIsRoot;

   if (!view2.fParentView)
      return Ancestry::ancestorIsRoot;
   
   BuildAncestryBranch(view1, branch1);
   BuildAncestryBranch(view2, branch2);
   
   QuartzView *ancestor = nil;
   
   for (unsigned i = 0, j = 0; i < view1.fLevel && j < view2.fLevel && branch1[i] == branch2[j]; ++i, ++j)
      ancestor = branch1[i];

   if (ancestor) {
      *lca = ancestor;
      return Ancestry::haveNonRootAncestor;
   }
   
   return Ancestry::ancestorIsRoot;
}

//______________________________________________________________________________
QuartzView *FindViewToPropagateEvent(QuartzView *viewFrom, Mask_t checkMask)
{
   //This function does not check passive grabs.
   assert(viewFrom != nil && "FindViewToPropagateEvent, view parameter is nil");
   
   if (viewFrom.fEventMask & checkMask)
      return viewFrom;
   
   for (viewFrom = viewFrom.fParentView; viewFrom; viewFrom = viewFrom.fParentView) {
      if (viewFrom.fEventMask & checkMask)
         return viewFrom;
   }

   return nil;
}

//______________________________________________________________________________
std::pair<QuartzView *, PointerGrab> FindGrabView(QuartzView *fromView, NSEvent *theEvent, EMouseButton btn)
{
   assert(fromView != nil && "FindGrabView, view parameter is nil");
   assert(theEvent != nil && "FindGrabView, event parameter is nil");

   const unsigned keyModifiers = Detail::GetKeyboardModifiersFromCocoaEvent(theEvent);
   
   QuartzView *grabView = 0;
   QuartzView *buttonPressView = 0;
   
   for (QuartzView *view = fromView; view != nil; view = view.fParentView) {
      //Top-first view to receive button press event.
      if (!buttonPressView && (view.fEventMask & kButtonPressMask))
         buttonPressView = view;

      //Top-last view with passive grab.
      if (view.fGrabButton == kAnyButton || view.fGrabButton == btn) {
         //Check modifiers.
         if (view.fGrabKeyModifiers == kAnyModifier || (view.fGrabKeyModifiers & keyModifiers))
            grabView = view;
      }
   }
   
   if (grabView)
      return std::make_pair(grabView, PointerGrab::passiveGrab);
   if (buttonPressView)
      return std::make_pair(buttonPressView, PointerGrab::implicitGrab);

   return std::make_pair((QuartzView *)0, PointerGrab::implicitGrab); 
}

//Aux. 'low-level' functions to generate events and call HandleEvent for a root window.

//______________________________________________________________________________
void SendEnterEvent(QuartzView *view, NSEvent *theEvent, EXMagic detail)
{
   //1. Parameters are valid.
   //2. view.fID is valid.
   //3. A window for view.fID exists.
   //This view must receive enter notify, I do not check it here.

   assert(view != nil && "SendEnterEvent, view parameter is nil");
   assert(theEvent != nil && "SendEnterEvent, event parameter is nil");
   assert(view.fID != 0 && "SendEnterEvent, view.fID is 0");

   TGWindow *window = gClient->GetWindowById(view.fID);
   assert(window != nullptr && "SendEnterEvent, window was not found");

   Event_t enterEvent = NewX11EventFromCocoaEvent(view.fID, theEvent);
   enterEvent.fType = kEnterNotify;
   enterEvent.fCode = detail;
   enterEvent.fState = GetModifiersFromCocoaEvent(theEvent);
   //Coordinates. Event possible happend not in a view,
   //but window should be the same. Also, coordinates are always
   //inside a view.
   
   ConvertEventLocationToROOTXY(theEvent, view, &enterEvent);
   

   //Dispatch:
   window->HandleEvent(&enterEvent);
}

//______________________________________________________________________________
void SendLeaveEvent(QuartzView *view, NSEvent *theEvent, EXMagic detail)
{
   //1. Parameters are valid.
   //2. view.fID is valid.
   //3. A window for view.fID exists.
   //This window should receive leave event, I do not check it here.

   assert(view != nil && "SendLeaveEvent, view parameter is nil");
   assert(theEvent != nil && "SendLeaveEvent, event parameter is nil");
   assert(view.fID != 0 && "SendLeaveEvent, view.fID is 0");
   
   TGWindow *window = gClient->GetWindowById(view.fID);
   assert(window != nullptr && "SendLeaveEvent, window was not found");

   Event_t leaveEvent = NewX11EventFromCocoaEvent(view.fID, theEvent);
   leaveEvent.fType = kLeaveNotify;
   leaveEvent.fCode = detail;
   leaveEvent.fState = GetModifiersFromCocoaEvent(theEvent);
   //Coordinates. Event possibly happend not in a view, also, coordinates are out of
   //the view.
   ConvertEventLocationToROOTXY(theEvent, view, &leaveEvent);
   //Dispatch:
   window->HandleEvent(&leaveEvent);
}

//______________________________________________________________________________
void SendPointerMotionEvent(QuartzView *view, NSEvent *theEvent)
{
   //1. Parameters are valid.
   //2. view.fID is valid.
   //3. A window for view.fID exists.
   //View receives pointer motion events, I do not check this condition here.
   
   assert(view != nil && "SendPointerMotionEvent, view parameter is nil");
   assert(theEvent != nil && "SendPointerMotionEvent, event parameter is nil");
   assert(view.fID != 0 && "SendPointerMotionEvent, view.fID is 0");
   
   TGWindow *window = gClient->GetWindowById(view.fID);
   assert(window != nullptr && "SendPointerMotionEvent, no window found");
   
   Event_t motionEvent = NewX11EventFromCocoaEvent(view.fID, theEvent);
   motionEvent.fType = kMotionNotify;
   motionEvent.fState = GetKeyboardModifiersFromCocoaEvent(theEvent);
   
   ConvertEventLocationToROOTXY(theEvent, view, &motionEvent);
   //Dispatch:
   window->HandleEvent(&motionEvent);
}

//______________________________________________________________________________
void SendButtonPressEvent(QuartzView *view, NSEvent *theEvent, EMouseButton btn)
{
   //1. Parameters are valid.
   //2. view.fID is valid.
   //3. A window for view.fID exists.
   //View receives this event (either grab or select input) 
   //   - I do not check this condition here.

   assert(view != nil && "SendButtonPressEvent, view parameter is nil");
   assert(theEvent != nil && "SendButtonPressEvent, event parameter is nil");
   assert(view.fID != 0 && "SendButtonPressEvent, view.fID is 0");
   
   TGWindow *window = gClient->GetWindowById(view.fID);
   assert(window != nullptr && "SendButtonPressEvent, window was not found");

   Event_t pressEvent = NewX11EventFromCocoaEvent(view.fID, theEvent);
   pressEvent.fType = kButtonPress;
   pressEvent.fCode = btn;
   pressEvent.fState = GetKeyboardModifiersFromCocoaEvent(theEvent);
   //
   ConvertEventLocationToROOTXY(theEvent, view, &pressEvent);
   //
   //Dispatch:
   window->HandleEvent(&pressEvent);
}

//______________________________________________________________________________
void SendButtonReleaseEvent(QuartzView *view, NSEvent *theEvent, EMouseButton btn)
{
   //1. Parameters are valid.
   //2. view.fID is valid.
   //3. A window for view.fID exists.
   //View must button release events, I do not check this here.

   assert(view != nil && "SendButtonReleaseEvent, view parameter is nil");
   assert(theEvent != nil && "SendButtonReleaseEvent, event parameter is nil");
   assert(view.fID != 0 && "SendButtonReleaseEvent, view.fID is 0");
   
   TGWindow *window = gClient->GetWindowById(view.fID);
   assert(window != nullptr && "SendButtonReleaseEvent, window was not found");
 
   Event_t releaseEvent = NewX11EventFromCocoaEvent(view.fID, theEvent);
   releaseEvent.fType = kButtonRelease;
   releaseEvent.fCode = btn;
   releaseEvent.fState = GetKeyboardModifiersFromCocoaEvent(theEvent);
   //
   ConvertEventLocationToROOTXY(theEvent, view, &releaseEvent);
   //
   window->HandleEvent(&releaseEvent);
}

//Aux. functions to send events to view's branch.

//______________________________________________________________________________
void SendEnterEventRange(QuartzView *from, QuartzView *to, NSEvent *theEvent, EXMagic mode)
{
   //[from, to) - legal range, 'to' must be ancestor for 'from'.
   assert(from != nil && "SendEnterEventRange, 'from' parameter is nil");
   assert(to != nil && "SendEnterEventRange, 'to' parameter is nil");
   assert(theEvent != nil && "SendEnterEventRange, event parameter is nil");
   
   while (from != to) {
      if (from.fEventMask & kEnterWindowMask)
         SendEnterEvent(from, theEvent, mode);
      from = from.fParentView;
   }
}

//______________________________________________________________________________
void SendEnterEventClosedRange(QuartzView *from, QuartzView *to, NSEvent *theEvent, EXMagic mode)
{
   //[from, to] - inclusive, legal range, 'to' must be ancestor for 'from'.
   assert(from != nil && "SendEnterEventClosedRange, 'from' parameter is nil");
   assert(to != nil && "SendEnterEventClosedRange, 'to' parameter is nil");
   assert(theEvent != nil && "SendEnterEventClosedRange, event parameter is nil");
   
   SendEnterEventRange(from, to, theEvent, mode);
   if (to.fEventMask & kEnterWindowMask)
      SendEnterEvent(to, theEvent, mode);
}

//______________________________________________________________________________
void SendLeaveEventRange(QuartzView *from, QuartzView *to, NSEvent *theEvent, EXMagic mode)
{
   //[from, to) - legal range, 'to' must be ancestor for 'from'.
   assert(from != nil && "SendLeaveEventRange, 'from' parameter is nil");
   assert(to != nil && "SendLeaveEventRange, 'to' parameter is nil");
   assert(theEvent != nil && "SendLeaveEventRange, event parameter is nil");

   while (from != to) {
      if (from.fEventMask & kLeaveWindowMask)
         SendLeaveEvent(from, theEvent, mode);
      from = from.fParentView;
   }
}

//______________________________________________________________________________
void SendLeaveEventClosedRange(QuartzView *from, QuartzView *to, NSEvent *theEvent, EXMagic mode)
{
   //[from, to] - inclusive, legal range, 'to' must be ancestor for 'from'.
   assert(from != nil && "SendLeaveEventClosedRange, 'from' parameter is nil");
   assert(to != nil && "SendLeaveEventClosedRange, 'to' parameter is nil");
   assert(theEvent != nil && "SendLeaveEventClosedRange, event parameter is nil");

   SendLeaveEventRange(from, to, theEvent, mode);
   if (to.fEventMask & kLeaveWindowMask)
      SendLeaveEvent(to, theEvent, mode);
}

//Top-level crossing event generators.

//______________________________________________________________________________
void GenerateCrossingEventChildToParent(QuartzView *parent, QuartzView *child, NSEvent *theEvent, EXMagic detail)
{
   //Pointer moves from window A to window B and A is an inferior of B.
   //Generate LeaveNotify on A (with detail NotifyAncestor).
   //Generate LeaveNotify for every window between A and B, exclusive (with detail NotifyVirtual)
   //Generate EnterNotify for B with detail NotifyInferior.
   
   //ROOT does not have NotifyAncestor/NotifyInferior.
   
   assert(parent != nil && "GenerateCrossingEventChildToParent, parent parameter is nil");
   assert(child != nil && "GenerateCrossingEventChildToParent, child parameter is nil");
   assert(theEvent != nil && "GenerateCrossingEventChildToParent, event parameter is nil");
   assert(child.fParentView != nil && "GenerateCrossingEventChildToParent, child parameter must have QuartzView* parent");
   
   if (child.fEventMask & kLeaveWindowMask)
      SendLeaveEvent(child, theEvent, detail);

   SendLeaveEventRange(child.fParentView, parent, theEvent, detail);
   
   if (parent.fEventMask & kEnterWindowMask)
      SendEnterEvent(parent, theEvent, detail);
}

//______________________________________________________________________________
void GenerateCrossingEventParentToChild(QuartzView *parent, QuartzView *child, NSEvent *theEvent, EXMagic detail)
{
   //Pointer moves from window A to window B and B is an inferior of A.
   //Generate LeaveNotify event for A, detail == NotifyInferior.
   //Generate EnterNotify for each window between window A and window B, exclusive, detail == NotifyVirtual (no such entity in ROOT).
   //Generate EnterNotify on window B, detail == NotifyAncestor.
   
   //ROOT does not have NotifyInferior/NotifyAncestor.
   
   assert(parent != nil && "GenerateCrossingEventParentToChild, parent parameter is nil");
   assert(child != nil && "GenerateCrossingEventParentToChild, child parameter is nil");
   assert(theEvent != nil && "GenerateCrossingEventParentToChild, event parameter is nil");
   assert(child.fParentView != nil && "GenerateCrossingEventParentToChild, child parameter must have QuartzView* parent");
   
   if (parent.fEventMask & kLeaveWindowMask)
      SendLeaveEvent(parent, theEvent, detail);

   //I do not know, if the order must be reversed, but if yes - it's already FAR TOO
   //expensive to do (but I'll reuse my 'branch' arrays from  FindLowestAncestor).
   SendEnterEventRange(child.fParentView, parent, theEvent, detail);
   
   if (child.fEventMask & kEnterWindowMask)
      SendEnterEvent(child, theEvent, detail);
}

//______________________________________________________________________________
void GenerateCrossingEventFromChild1ToChild2(QuartzView *child1, QuartzView *child2, QuartzView *ancestor, NSEvent *theEvent, EXMagic detail)
{
   //Pointer moves from window A to window B and window C is their lowest common ancestor.
   //Generate LeaveNotify for window A with detail == NotifyNonlinear.
   //Generate LeaveNotify for each window between A and C, exclusive, with detail == NotifyNonlinearVirtual
   //Generate EnterNotify (detail == NotifyNonlinearVirtual) for each window between C and B, exclusive
   //Generate EnterNotify for window B, with detail == NotifyNonlinear.
   assert(child1 != nil && "GenerateCrossingEventFromChild1ToChild2, child1 parameter is nil");
   assert(child2 != nil && "GenerateCrossingEventFromChild1ToChild2, child2 parameter is nil");
   assert(theEvent != nil && "GenerateCrossingEventFromChild1ToChild2, theEvent parameter is nil");
   
   //ROOT does not have NotifyNonlinear/NotifyNonlinearVirtual.
   
   if (child1.fEventMask & kLeaveWindowMask)
      SendLeaveEvent(child1, theEvent, detail);
   
   if (!ancestor) {
      //From child1 to it's top-level view.
      if (child1.fParentView)
         SendLeaveEventClosedRange(child1.fParentView, (QuartzView *)[[child1 window] contentView], theEvent, detail);
      if (child2.fParentView)
         SendEnterEventClosedRange(child2.fParentView, (QuartzView *)[[child2 window] contentView], theEvent, detail);
   } else {
      if (child1.fParentView)
         SendLeaveEventRange(child1.fParentView, ancestor, theEvent, detail);
      if (child2.fParentView)
         SendEnterEventRange(child2.fParentView, ancestor, theEvent, detail);
   }

   if (child2.fEventMask & kEnterWindowMask)
      SendEnterEvent(child2, theEvent, detail);
}

}//Detail

//______________________________________________________________________________
EventTranslator::EventTranslator()
                     : fViewUnderPointer(nil),
                       fPointerGrab(PointerGrab::noGrab),
                       //fButtonPressed(0), //0 == kAnyButton
                       fCurrentGrabView(nil)
{
}

//______________________________________________________________________________
void EventTranslator::GenerateCrossingEvent(QuartzView *view, NSEvent *theEvent)
{
   //View parameter can be nil (we exit any window).
   assert(theEvent != nil && "GenerateCrossingEvent, event parameter is nil");

   if (fPointerGrab == PointerGrab::noGrab) {
      NSView *candidateView = [[[view window] contentView] hitTest : [theEvent locationInWindow]];
      if (candidateView && ![candidateView isKindOfClass : [QuartzView class]]) {
         NSLog(@"EventTranslator::GenerateCrossingEvent: error, hit test returned not a QuartzView!");
         candidateView = nil;
      }

      GenerateCrossingEvent((QuartzView *)candidateView, theEvent, kNotifyNormal);
   } else if (fCurrentGrabView) {
      if (fCurrentGrabView.fOwnerEvents) {
         NSView *candidateView = [[[view window] contentView] hitTest : [theEvent locationInWindow]];
         if (candidateView && ![candidateView isKindOfClass : [QuartzView class]]) {
            NSLog(@"EventTranslator::GenerateCrossingEvent: error, hit test returned not a QuartzView!");
            candidateView = nil;
         }
         
         GenerateCrossingEvent((QuartzView *)candidateView, theEvent, kNotifyNormal);
      } else {
         GenerateCrossingEvent(fCurrentGrabView, theEvent, kNotifyNormal);
      }
   }
}

//______________________________________________________________________________
bool EventTranslator::HasPointerGrab()const
{
   return fPointerGrab != PointerGrab::noGrab;
}

//______________________________________________________________________________
void EventTranslator::GenerateCrossingEvent(QuartzView *view, NSEvent *theEvent, EXMagic detail)
{
   assert(theEvent != nil && "GenerateCrossingEvent, event parameter is nil");

   if (view == fViewUnderPointer) {
      //This can happen: tracking areas for stacked windows call
      //mouseExited even for overlapped views (so you have a bunch of mouseExited/mouseEntered
      //for one cursor move). In mouseEntered/mouseExited
      //I'm looking for the top level view under cursor and try to generate cross event
      //for this view only.
      return;
   }

   if (!fViewUnderPointer) {
      //We enter window "from the screen" - do not leave any window.
      //Send EnterNotify event.
      if (view)//Check, if order is OK.
         Detail::SendEnterEventClosedRange(view, (QuartzView *)[[view window] contentView], theEvent, detail);
   } else if (!view) {
      //We exit all views. Order must be OK here.
      Detail::SendLeaveEventClosedRange(fViewUnderPointer, (QuartzView *)[[fViewUnderPointer window] contentView], theEvent, detail);
   } else {
      QuartzView *ancestor = 0;
      Ancestry rel = FindRelation(fViewUnderPointer, view, &ancestor);
      if (rel == Ancestry::view1IsParent) {
         //Case 1.
         //From A to B.
         //_________________
         //| A              |
         //|   |---------|  |
         //|   |  B      |  |
         //|   |         |  |
         //|   |---------|  |
         //|                |
         //|________________|
         Detail::GenerateCrossingEventParentToChild(fViewUnderPointer, view, theEvent, detail);
      } else if (rel == Ancestry::view2IsParent) {
         //Case 2.
         //From A to B.
         //_________________
         //| B              |
         //|   |---------|  |
         //|   |  A      |  |
         //|   |         |  |
         //|   |---------|  |
         //|                |
         //|________________|   
         Detail::GenerateCrossingEventChildToParent(view, fViewUnderPointer, theEvent, detail);
      } else {
         //Case 3.
         //|--------------------------------|
         //| C   |------|      |-------|    |
         //|     | A    |      | B     |    |
         //|     |______|      |_______|    |
         //|________________________________|
         //Ancestor is either some view, or 'root' window.
         //The fourth case (different screens) is not implemented (and I do not know, if I want to implement it).
         Detail::GenerateCrossingEventFromChild1ToChild2(fViewUnderPointer, view, ancestor, theEvent, detail);
      }
   }
   
   fViewUnderPointer = view;
}

//______________________________________________________________________________
void EventTranslator::GeneratePointerMotionEvent(QuartzView *eventView, NSEvent *theEvent)
{
   assert(eventView != nil && "GeneratePointerMotionEvent, view parameter is nil");
   assert(theEvent != nil && "GeneratePointerMotionEvent, event parameter is nil");

   if (fPointerGrab == PointerGrab::noGrab) {
      return GeneratePointerMotionEventNoGrab(eventView, theEvent);
   } else {
      if (fCurrentGrabView) {
         if (fCurrentGrabView.fOwnerEvents) {
            //The problem here is that Cocoa has it's own grabs.
            if (QuartzView *candidateView = Detail::FindViewToPropagateEvent(eventView, kPointerMotionMask)) {
               Detail::SendPointerMotionEvent(candidateView, theEvent);
            }
         } else {
            Detail::SendPointerMotionEvent(fCurrentGrabView, theEvent);
         }
      }
   }
}

//______________________________________________________________________________
void EventTranslator::GenerateButtonPressEvent(QuartzView *eventView, NSEvent *theEvent, EMouseButton btn)
{
   assert(eventView != nil && "GenerateMouseDownEvent, view parameter is nil");
   assert(theEvent != nil && "GenerateMouseDownEvent, event parameter is nil");
   
   if (fPointerGrab == PointerGrab::noGrab)
      return GenerateButtonPressEventNoGrab(eventView, theEvent, btn);
   else {
      //X11 spec. says NOTHING about the case when I have two passive grabs on window A and window B,
      //say left button on A and right button on B. What should happen if I press left button in A, move to
      //B and press the right button? In my test programm on X11 (Ubuntu) I can see, that now they BOTH
      //are active grabs. I'm not going to implement this mess, unless I have a correct formal description.
      if (fCurrentGrabView) {
         if (fCurrentGrabView.fOwnerEvents) {
            if (QuartzView *view = Detail::FindViewToPropagateEvent(eventView, kButtonPressMask))
               Detail::SendButtonPressEvent(view, theEvent, btn);
         } else 
            Detail::SendButtonPressEvent(fCurrentGrabView, theEvent, btn);
      }
   }
}

//______________________________________________________________________________
void EventTranslator::GenerateButtonReleaseEvent(QuartzView *eventView, NSEvent *theEvent, EMouseButton btn)
{
   assert(eventView != nil && "GenerateButtonReleaseEvent, view parameter is nil");
   assert(theEvent != nil && "GenerateButtonReleaseEvent, event parameter is nil");
   
   if (fPointerGrab == PointerGrab::implicitGrab) {
      GenerateButtonReleaseEventImplicitGrab(theEvent, btn);
      //Crossing event
      GenerateCrossingEvent(eventView, theEvent, kNotifyUngrab);
      //Cancel grab.
      fPointerGrab = PointerGrab::noGrab;
      fCurrentGrabView = nil;
   } else if (fPointerGrab == PointerGrab::passiveGrab) {
      GenerateButtonReleaseEventActiveGrab(eventView, theEvent, btn);
      //Crossing event.
      GenerateCrossingEvent(eventView, theEvent, kNotifyUngrab);
      //Cancel grab.
      fPointerGrab = PointerGrab::noGrab;
      fCurrentGrabView = nil;
   } else if (fPointerGrab == PointerGrab::activeGrab) {
      GenerateButtonReleaseEventActiveGrab(eventView, theEvent, btn);
   } else if (QuartzView *candidateView = Detail::FindViewToPropagateEvent(eventView, kButtonReleaseMask)) {
      Detail::SendButtonReleaseEvent(candidateView, theEvent, btn);
   }
}

//______________________________________________________________________________
void EventTranslator::SetPointerGrab(QuartzView *grabView)
{
   assert(grabView != nil && "SetPointerGrab, view parameter is nil");
   
   //Now some magic to receive mouse move events even outside any window.
   if (grabView.fGrabButtonEventMask & kPointerMotionMask)
      [[grabView window] setAcceptsMouseMovedEvents : YES];
      
   fCurrentGrabView = grabView;
   fPointerGrab = PointerGrab::activeGrab;
}

//______________________________________________________________________________
void EventTranslator::CancelPointerGrab(QuartzView *grabView)
{
   assert(grabView != nil && "CancelPointerGrab, view parameter is nil");
   assert(grabView == fCurrentGrabView && "CancelPointerGrab, view parameter is not a current grab view");
   //
   [[grabView window] setAcceptsMouseMovedEvents : NO];//Do not track mouse move events outside window anymore.
   
   fCurrentGrabView = nil;
   fPointerGrab = PointerGrab::noGrab;
}

//______________________________________________________________________________
void EventTranslator::GeneratePointerMotionEventNoGrab(QuartzView *eventView, NSEvent *theEvent)
{
   //The problem is that mouse motion events come not only to the view on the top of a stack,
   //but also to every view which is under cursor (even if it's overlapped).
   //I do not know, why apple did this. For me - it's ridiculous. Also, I have to think
   //about event propagation and all this makes things even more complex.

   assert(eventView != nil && "GeneratePointerMotionEventNoGrab, view parameter is nil");
   assert(theEvent != nil && "GeneratePointerMotionEventNoGrab, event parameter is nil");
   
   //Find view on the top of stack:
   QuartzView *candidateView = (QuartzView *)[[[eventView window] contentView] hitTest : [theEvent locationInWindow]];
   if (candidateView) {
      //Do propagation.
      candidateView = Detail::FindViewToPropagateEvent(candidateView, kPointerMotionMask);
      if (candidateView)
         Detail::SendPointerMotionEvent(candidateView, theEvent);
   }
}

//______________________________________________________________________________
void EventTranslator::GenerateButtonPressEventNoGrab(QuartzView *view, NSEvent *theEvent, EMouseButton btn)
{
   assert(view != nil && "GenerateButtonPressEventNoGrab, view parameter is nil");
   assert(theEvent != nil && "GenerateButtonPressEventNoGrab, event parameter is nil");

   const auto grab = Detail::FindGrabView(view, theEvent, btn);
   fCurrentGrabView = grab.first;
   fPointerGrab = grab.second;

   //And now something badly defined. I tried X11 on mac and on linux, they do different things.
   //I'll do what was said in a spec and I do not care, if it's right or not, since there
   //is nothing 'right' in all this crap and mess. Since I'm activating grab,
   //before I send ButtonPress event, I'll send leave/enter notify events, if this is
   //required (previously entered view and current view are different).
   //If nothing was selected, on linux it looks like 'root' window
   //becomes a grab and all pointer events are discarded until ungrab.
   GenerateCrossingEvent(fCurrentGrabView, theEvent, kNotifyGrab);
   
   if (fCurrentGrabView)
      Detail::SendButtonPressEvent(fCurrentGrabView, theEvent, btn);
}

//______________________________________________________________________________
void EventTranslator::GenerateButtonReleaseEventImplicitGrab(NSEvent *theEvent, EMouseButton btn)
{
   assert(theEvent != nil && "GenerateButtonReleaseEventImplicitGrab, event parameter is nil");

   if (fCurrentGrabView) {
      if (fCurrentGrabView.fGrabButtonEventMask & kButtonReleaseMask)
         Detail::SendButtonReleaseEvent(fCurrentGrabView, theEvent, btn);
   }
}

//______________________________________________________________________________
void EventTranslator::GenerateButtonReleaseEventActiveGrab(QuartzView *eventView, NSEvent *theEvent, EMouseButton btn)
{
   assert(eventView != nil && "GenerateButtonReleaseEventActiveGrab, view parameter is nil");
   assert(theEvent != nil && "GenerateButtonReleaseEventActiveGrab, event parameter is nil");
   assert(fCurrentGrabView != nil && "GenerateButtonReleaseEventActiveGrab, current grab view is nil");//TODO:?

   if (!fCurrentGrabView.fOwnerEvents) {
      if ((fCurrentGrabView.fGrabButtonEventMask & kButtonReleaseMask))
         Detail::SendButtonReleaseEvent(fCurrentGrabView, theEvent, btn);
   } else {
      if (fCurrentGrabView.fGrabButtonEventMask & kButtonReleaseMask)
         Detail::SendButtonReleaseEvent(fCurrentGrabView, theEvent, btn);
      else if (QuartzView *candidateView = Detail::FindViewToPropagateEvent(eventView, kButtonReleaseMask))
         Detail::SendButtonReleaseEvent(candidateView, theEvent, btn);
   }
}

//______________________________________________________________________________
Ancestry EventTranslator::FindRelation(QuartzView *view1, QuartzView *view2, QuartzView **lca)
{
   assert(view1 != nil && "FindRelation, view1 parameter is nil");
   assert(view2 != nil && "FindRelation, view2 parameter is nil");
   assert(lca != nullptr && "FindRelation, lca parameter is nil");
   
   if (Detail::IsParent(view1, view2)) 
      return Ancestry::view1IsParent;
   
   if (Detail::IsParent(view2, view1))
      return Ancestry::view2IsParent;
   
   return Detail::FindLowestCommonAncestor(view1, fBranch1, view2, fBranch2, lca);
}

//______________________________________________________________________________
void EventTranslator::SortTopLevelWindows()
{
   const ROOT::MacOSX::Util::AutoreleasePool pool;

   fWindowStack.clear();

   NSArray *orderedWindows = [NSApp orderedWindows];
   for (NSWindow *window in orderedWindows) {
      if (![window isKindOfClass : [QuartzWindow class]])
         continue;
      fWindowStack.push_back((QuartzWindow *)window);
   }
}

}//X11
}//MacOSX
}//ROOT
