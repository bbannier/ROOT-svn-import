#include <stdexcept>
#include <cassert>

#include <Cocoa/Cocoa.h>

#include "CocoaPrivate.h"
#include "CocoaUtils.h"

namespace ROOT {
namespace MacOSX {
namespace Details {

//______________________________________________________________________________
CocoaPrivate::CocoaPrivate()
               : fCurrentWindowID(1)//0 is for 'ROOT', any real window has id > 0.
{
   //Init NSApplication, if it was not done yet.
   Util::AutoreleasePool pool;
   [NSApplication sharedApplication];
}

//______________________________________________________________________________
CocoaPrivate::~CocoaPrivate()
{
}

//______________________________________________________________________________
unsigned CocoaPrivate::RegisterWindow(NSObject *nsWin)
{
   //In case of X11, gVirtualX->CreateWindow returns some 'descriptor' (integer number),
   //which is valid for X11 calls and window can be identified by this descriptor.
   //With Cocoa, we have NSWindow pointers, which can not be simply returned from CreateWindow.
   //So I need some mapping between real NSObjects and ROOT's integers.
   //I have an internal numbering - just subsequent numbers. NSWindow has -windowNumber
   //method - I'll probably use it in future, but still the machinery will be almost the same.
   //If window is closed, it's id will go to fFreeWindowIDs array. So, check it first:
   
   unsigned newID = fCurrentWindowID;

   if (fFreeWindowIDs.size()) {
      newID = fFreeWindowIDs.back();
      fFreeWindowIDs.pop_back();
   } else
      fCurrentWindowID++;

   assert(fWindows.find(newID) == fWindows.end() && "RegisterWindow, id for new window is still in use");

   fWindows[newID] = nsWin;

   return newID;
}

//______________________________________________________________________________
id<X11Drawable> CocoaPrivate::GetWindow(unsigned winID)const
{
   auto winIter = fWindows.find(winID);

   assert(winIter != fWindows.end() && "GetWindow, non-existing window requested");

   return (id<X11Drawable>)winIter->second.Get();
}

//______________________________________________________________________________
void CocoaPrivate::DeleteWindow(unsigned winID)
{
   auto winIter = fWindows.find(winID);

   assert(winIter != fWindows.end() && "DelteWindow, non existing winID");
   
   //Probably, I'll need some additional cleanup here later. Now just delete NSObject and
   //reuse its id.
   id window = winIter->second.Get();
   if ([window respondsToSelector : @selector(removeFromSuperview)])
      [window removeFromSuperview];

   fFreeWindowIDs.push_back(winID);
   fWindows.erase(winIter);//StrongReference should do work here.
}

}
}
}
