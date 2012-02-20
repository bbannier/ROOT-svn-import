#include <algorithm>
#include <utility>
#include <cassert>

#include <Cocoa/Cocoa.h>

#include "QuartzWindow.h"
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
   //2. Both event and view must be in the same window.

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

   unsigned rootKeyModifiers = 0;//root == ROOT :)

   const NSUInteger modifiers = [theEvent modifierFlags];
  
   if (modifiers & NSAlphaShiftKeyMask)
      rootKeyModifiers |= kKeyLockMask;
   if (modifiers & NSShiftKeyMask)
      rootKeyModifiers |= kKeyShiftMask;
   if (modifiers & NSControlKeyMask)
      rootKeyModifiers |= kKeyControlMask;
   if (modifiers & NSAlternateKeyMask)
      rootKeyModifiers |= kKeyMod1Mask;
   if (modifiers & NSCommandKeyMask)
      rootKeyModifiers |= kKeyMod2Mask;

   return rootKeyModifiers;
}

//Aux. functions to generate events and call HandleEvent for a root window.

//______________________________________________________________________________
void SendEnterEvent(QuartzView *view, NSEvent *theEvent, EXMagic detail)
{
   //1. Parameters are valid.
   //2. view.fID is valid.

   assert(view != nil && "SendEnterEvent, view parameter is nil");
   assert(theEvent != nil && "SendEnterEvent, event parameter is nil");

   TGWindow *window = gClient->GetWindowById(view.fID);
   assert(window != nullptr && "SendEnterEvent, window was not found");

   if (view.fEventMask & kEnterWindowMask) {
      //NSLog(@"EnterNotify for %u", view.fID);
      Event_t enterEvent = NewX11EventFromCocoaEvent(view.fID, theEvent);
      enterEvent.fType = kEnterNotify;
      //Coordinates. Event possible happend not in a view,
      //but window should be the same. Also, coordinates are always
      //inside a view.
      ConvertEventLocationToROOTXY(theEvent, view, &enterEvent);
      //NSLog(@"Enter %u at %d %d", view.fID, enterEvent.fX, enterEvent.fY);
      
      enterEvent.fCode = detail;
      //Deliver!
      window->HandleEvent(&enterEvent);
   }
}

//______________________________________________________________________________
void SendLeaveEvent(QuartzView *view, NSEvent *theEvent, EXMagic detail)
{
   //1. Parameters are valid.
   //2. view.fID is valid.

   assert(view != nil && "SendLeaveEvent, view parameter is nil");
   assert(theEvent != nil && "SendLeaveEvent, event parameter is nil");
   
   TGWindow *window = gClient->GetWindowById(view.fID);
   assert(window != nullptr && "SendLeaveEvent, window was not found");

   if (view.fEventMask & kLeaveWindowMask) {
      //NSLog(@"LeaveNotify for %u", view.fID);
      //Call HandleEvent here.
      Event_t leaveEvent = NewX11EventFromCocoaEvent(view.fID, theEvent);
      leaveEvent.fType = kLeaveNotify;
      //Coordinates. Event possible happend not in a view, also, coordinates are out of
      //the view.
      ConvertEventLocationToROOTXY(theEvent, view, &leaveEvent);
      //NSLog(@"Leave %u at %d %d", view.fID, leaveEvent.fX, leaveEvent.fY);

      leaveEvent.fCode = detail;
      //Deliver!
      window->HandleEvent(&leaveEvent);
   }
}

//______________________________________________________________________________
void SendMousePressEvent(QuartzView *, NSEvent *, EMouseButton)
{
   //1. Parameters are valid.
   //2. view.fID is valid.
   //3. View receives this event (either grab or select input).
/*
   assert(view != nil && "SendMousePressEvent, view parameter is nil");
   assert(theEvent != nil && "SendMousePressEvent, event parameter is nil");

   TGWindow *window = gClient->GetWindowById(view.fID);
   assert(window != nullptr && "SendMousePressEvent, window was not found");

   Event_t enterEvent = NewX11EventFromCocoaEvent(view.fID, theEvent);
   enterEvent.fType = kButtonPress;
   //
   ConvertEventLocationToROOTXY(theEvent, view, &enterEvent);
   //Deliver!
   window->HandleEvent(&enterEvent);*/
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
   SendLeaveEvent(to, theEvent, mode);
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
         if (view.fGrabKeyModifiers & keyModifiers)
            grabView = view;
      }
   }
   
   if (grabView)
      return std::make_pair(grabView, PointerGrab::passiveGrab);
   if (buttonPressView)
      return std::make_pair(buttonPressView, PointerGrab::implicitGrab);

   return std::make_pair((QuartzView *)0, PointerGrab::implicitGrab); 
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
   
   assert(parent != nil && "GenerateCrossingEventNormalChildToParent, parent parameter is nil");
   assert(child != nil && "GenerateCrossingEventNormalChildToParent, child parameter is nil");
   assert(theEvent != nil && "GenerateCrossingEventNormalChildToParent, event parameter is nil");
   assert(child.fParentView != nil && "GenerateCrossingEventNormalChildToParent, child parameter must have QuartzView* parent");
   
   SendLeaveEvent(child, theEvent, detail);

   SendLeaveEventRange(child.fParentView, parent, theEvent, detail);
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
   
   assert(parent != nil && "GenerateCrossingEventNormalParentToChild, parent parameter is nil");
   assert(child != nil && "GenerateCrossingEventNormalParentToChild, child parameter is nil");
   assert(theEvent != nil && "GenerateCrossingEventNormalParentToChild, event parameter is nil");
   assert(child.fParentView != nil && "GenerateCrossingEventNormalParentToChild, child parameter must have QuartzView* parent");
   
   SendLeaveEvent(parent, theEvent, detail);

   //I do not know, if the order must be reversed, but if yes - it's already FAR TOO
   //expensive to do (but I'll reuse my 'branch' arrays from  FindLowestAncestor).
   SendEnterEventRange(child.fParentView, parent, theEvent, detail);
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
   assert(child1 != nil && "GenerateCrossingEventNormalFromChild1ToChild2, child1 parameter is nil");
   assert(child2 != nil && "GenerateCrossingEventNormalFromChild1ToChild2, child2 parameter is nil");
   assert(theEvent != nil && "GenerateCrossingEventNormalFromChild1ToChild2, theEvent parameter is nil");
   
   //ROOT does not have NotifyNonlinear/NotifyNonlinearVirtual.
   
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
      const NSPoint windowPoint = [theEvent locationInWindow];
      NSView *candidateView = [[[view window] contentView] hitTest : windowPoint];
      if (candidateView && ![candidateView isKindOfClass : [QuartzView class]]) {
         NSLog(@"EventTranslator::Error: hit test returned not a QuartzView!");
         candidateView = nil;
      }

      GenerateCrossingEvent(fViewUnderPointer, (QuartzView *)candidateView, theEvent, kNotifyNormal);
      fViewUnderPointer = (QuartzView *)candidateView;
   } else {
      //
   }
}

//______________________________________________________________________________
bool EventTranslator::HasPointerGrab()const
{
   return fPointerGrab != PointerGrab::noGrab;
}

//______________________________________________________________________________
void EventTranslator::GenerateCrossingEvent(QuartzView *viewUnderPointer, QuartzView *view, NSEvent *theEvent, EXMagic detail)
{
   assert(theEvent != nil && "GenerateCrossingEventNormal, event parameter is nil");

   if (view == viewUnderPointer) {
      //This can happen: tracking areas for stacked windows call
      //mouseExited even for overlapped views (so you have a bunch of mouseExited/mouseEntered
      //for one cursor move). In mouseEntered/mouseExited
      //I'm looking for the top level view under cursor and try to generate cross event
      //for this view only.
      return;
   }

   if (!viewUnderPointer) {
      //We enter window "from the screen" - do not leave any window.
      //Send EnterNotify event.
      if (view)//Check, if order is OK.
         Detail::SendEnterEventClosedRange(view, (QuartzView *)[[view window] contentView], theEvent, detail);
   } else if (!view) {
      //We exit all views. Order must be OK here.
      Detail::SendLeaveEventClosedRange(viewUnderPointer, (QuartzView *)[[fViewUnderPointer window] contentView], theEvent, detail);
   } else {
      QuartzView *ancestor = 0;
      Ancestry rel = FindRelation(viewUnderPointer, view, &ancestor);
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
         Detail::GenerateCrossingEventParentToChild(viewUnderPointer, view, theEvent, detail);
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
         Detail::GenerateCrossingEventChildToParent(view, viewUnderPointer, theEvent, detail);
      } else {
         //Case 3.
         //|--------------------------------|
         //| C   |------|      |-------|    |
         //|     | A    |      | B     |    |
         //|     |______|      |_______|    |
         //|________________________________|
         //Ancestor is either some view, or 'root' window.
         //The fourth case (different screens) is not implemented (and I do not know, if I want to implement it).
         Detail::GenerateCrossingEventFromChild1ToChild2(viewUnderPointer, view, ancestor, theEvent, detail);
      }
   }
}

//______________________________________________________________________________
void EventTranslator::GeneratePointerMotionEvent(QuartzView *eventView, NSEvent *theEvent)
{
   assert(eventView != nil && "GeneratePointerMotionEvent, view parameter is nil");
   assert(theEvent != nil && "GeneratePointerMotionEvent, event parameter is nil");

   if (fPointerGrab == PointerGrab::noGrab) {
      return GeneratePointerMotionEventNoGrab(eventView, theEvent);
   } else if (fPointerGrab == PointerGrab::implicitGrab) {
      //Actually, no need to separate?
   } else if (fPointerGrab == PointerGrab::activeGrab) {
      //Actually, no need to separate?   
   }
}

//______________________________________________________________________________
void EventTranslator::GenerateButtonPressEvent(QuartzView *eventView, NSEvent *theEvent, EMouseButton btn)
{
   assert(eventView != nil && "GenerateMouseDownEvent, view parameter is nil");
   assert(theEvent != nil && "GenerateMouseDownEvent, event parameter is nil");
   
   if (fPointerGrab == PointerGrab::noGrab)
      return GenerateButtonPressEventNoGrab(eventView, theEvent, btn);
   
   //else////
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
      if (candidateView) {
         assert(candidateView.fID != 0 && "GeneratePointerMotionEventNoGrab, view's fID is 0");
         
         TGWindow *window = gClient->GetWindowById(candidateView.fID);
         assert(window != nullptr && "GeneratePointerMotionEventNoGrab, no window was found");
         
         Event_t motionEvent = Detail::NewX11EventFromCocoaEvent(candidateView.fID, theEvent);
         motionEvent.fType = kMotionNotify;
         motionEvent.fState = 0;

         Detail::ConvertEventLocationToROOTXY(theEvent, candidateView, &motionEvent);

         window->HandleEvent(&motionEvent);
      }
   }
}

//______________________________________________________________________________
void EventTranslator::GenerateButtonPressEventNoGrab(QuartzView *view, NSEvent *theEvent, EMouseButton btn)
{
   assert(view != nil && "GenerateImplicitGrabEvents, view parameter is nil");
   assert(theEvent != nil && "GenerateImplicitGrabEvents, event parameter is nil");

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
   GenerateCrossingEvent(fViewUnderPointer, fCurrentGrabView, theEvent, kNotifyGrab);
   fViewUnderPointer = fCurrentGrabView;
   
   if (fCurrentGrabView) {
      
      assert(fCurrentGrabView.fID != 0 && "GenerateButtonPressEventNoGrab, grab view has fID == 0");
      TGWindow *window = gClient->GetWindowById(fCurrentGrabView.fID);
      assert(window != nullptr && "GenerateButtonpressEventNoGrab, grab window was not found");

      //
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



}//X11
}//MacOSX
}//ROOT
