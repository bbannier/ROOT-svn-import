// @(#)root/graf2d:$Id$
// Author: Timur Pocheptsov 5/12/2011

/*************************************************************************
 * Copyright (C) 1995-2012, Rene Brun and Fons Rademakers.               *
 * All rights reserved.                                                  *
 *                                                                       *
 * For the licensing terms see $ROOTSYS/LICENSE.                         *
 * For the list of contributors see $ROOTSYS/README/CREDITS.             *
 *************************************************************************/
 
#define NDEBUG

#include <stdexcept>
#include <vector>
#include <map>
#include <set>

#import <Cocoa/Cocoa.h>

#include "TSeqCollection.h"
#include "TMacOSXSystem.h"
#include "CocoaUtils.h"
#include "TVirtualX.h"
#include "TError.h"

@interface RunStopper : NSObject
@end

@implementation RunStopper

//We attach this delegate only once, when trying to initialize NSApplication (by calling its -run method).
//______________________________________________________________________________
- (void) stopRun
{
   [NSApp stop : nil];
   //This is not enough to stop, from docs:
   //This method notifies the application that you want to exit the current run loop as soon as it finishes processing the current NSEvent object.
   //This method does not forcibly exit the current run loop. Instead it sets a flag that the application checks only after it finishes dispatching an actual event object.


   //I'm sending a fake event, to stop.
   NSEvent* stopEvent = [NSEvent otherEventWithType: NSApplicationDefined location: NSMakePoint(0,0) modifierFlags: 0 timestamp: 0.0
                                 windowNumber: 0 context: nil subtype: 0 data1: 0 data2: 0];
   [NSApp postEvent : stopEvent atStart: true];
}

@end

@interface MenuLoader : NSObject
{
}

@end

@implementation MenuLoader

+(void) populateMainMenu
{
	NSMenu * mainMenu = [[NSMenu alloc] initWithTitle:@"NSMainMenu"];
	
	NSMenuItem * menuItem;
	NSMenu * submenu;
	
	// The titles of the menu items are for identification purposes only and shouldn't be localized.
	// The strings in the menu bar come from the submenu titles,
	// except for the application menu, whose title is ignored at runtime.
	menuItem = [mainMenu addItemWithTitle:@"Apple" action:NULL keyEquivalent:@""];
	submenu = [[NSMenu alloc] initWithTitle:@"Apple"];
	[NSApp performSelector:@selector(setAppleMenu:) withObject:submenu];
	[self populateApplicationMenu:submenu];
	[mainMenu setSubmenu:submenu forItem:menuItem];
	
	menuItem = [mainMenu addItemWithTitle:@"File" action:NULL keyEquivalent:@""];
	submenu = [[NSMenu alloc] initWithTitle:NSLocalizedString(@"File", @"The File menu")];
	[self populateFileMenu:submenu];
	[mainMenu setSubmenu:submenu forItem:menuItem];
	
	menuItem = [mainMenu addItemWithTitle:@"Edit" action:NULL keyEquivalent:@""];
	submenu = [[NSMenu alloc] initWithTitle:NSLocalizedString(@"Edit", @"The Edit menu")];
	[self populateEditMenu:submenu];
	[mainMenu setSubmenu:submenu forItem:menuItem];
	
	/* TODO
	menuItem = [mainMenu addItemWithTitle:@"View" action:NULL keyEquivalent:@""];
	submenu = [[NSMenu alloc] initWithTitle:NSLocalizedString(@"View", @"The View menu")];
	[self populateViewMenu:submenu];
	[mainMenu setSubmenu:submenu forItem:menuItem];
	*/
	
	menuItem = [mainMenu addItemWithTitle:@"Window" action:NULL keyEquivalent:@""];
	submenu = [[NSMenu alloc] initWithTitle:NSLocalizedString(@"Window", @"The Window menu")];
	[self populateWindowMenu:submenu];
	[mainMenu setSubmenu:submenu forItem:menuItem];
	[NSApp setWindowsMenu:submenu];
	
	menuItem = [mainMenu addItemWithTitle:@"Help" action:NULL keyEquivalent:@""];
	submenu = [[NSMenu alloc] initWithTitle:NSLocalizedString(@"Help", @"The Help menu")];
	[self populateHelpMenu:submenu];
	[mainMenu setSubmenu:submenu forItem:menuItem];
	
	/* TODO
	menuItem = [mainMenu addItemWithTitle:@"Debug" action:NULL keyEquivalent:@""];
	submenu = [[NSMenu alloc] initWithTitle:NSLocalizedString(@"Debug", @"The Debug menu")];
	[self populateDebugMenu:submenu];
	[mainMenu setSubmenu:submenu forItem:menuItem];
	*/
	
	[NSApp setMainMenu:mainMenu];
   
   [NSMenu setMenuBarVisible:YES];
}

+(void) populateApplicationMenu:(NSMenu *)aMenu
{
	NSString * applicationName = @"root";
	NSMenuItem * menuItem;
	
	menuItem = [aMenu addItemWithTitle:[NSString stringWithFormat:@"%@ %@", NSLocalizedString(@"About", nil), applicationName]
								action:@selector(orderFrontStandardAboutPanel:)
						 keyEquivalent:@""];
	[menuItem setTarget:NSApp];
	
	[aMenu addItem:[NSMenuItem separatorItem]];
	
	menuItem = [aMenu addItemWithTitle:NSLocalizedString(@"Preferences...", nil)
								action:NULL
						 keyEquivalent:@","];
	
	[aMenu addItem:[NSMenuItem separatorItem]];
	
	menuItem = [aMenu addItemWithTitle:NSLocalizedString(@"Services", nil)
								action:NULL
						 keyEquivalent:@""];
	NSMenu * servicesMenu = [[NSMenu alloc] initWithTitle:@"Services"];
	[aMenu setSubmenu:servicesMenu forItem:menuItem];
	[NSApp setServicesMenu:servicesMenu];
	
	[aMenu addItem:[NSMenuItem separatorItem]];
	
	menuItem = [aMenu addItemWithTitle:[NSString stringWithFormat:@"%@ %@", NSLocalizedString(@"Hide", nil), applicationName]
								action:@selector(hide:)
						 keyEquivalent:@"h"];
	[menuItem setTarget:NSApp];
	
	menuItem = [aMenu addItemWithTitle:NSLocalizedString(@"Hide Others", nil)
								action:@selector(hideOtherApplications:)
						 keyEquivalent:@"h"];
	[menuItem setKeyEquivalentModifierMask:NSCommandKeyMask | NSAlternateKeyMask];
	[menuItem setTarget:NSApp];
	
	menuItem = [aMenu addItemWithTitle:NSLocalizedString(@"Show All", nil)
								action:@selector(unhideAllApplications:)
						 keyEquivalent:@""];
	[menuItem setTarget:NSApp];
	
	[aMenu addItem:[NSMenuItem separatorItem]];
	
	menuItem = [aMenu addItemWithTitle:[NSString stringWithFormat:@"%@ %@", NSLocalizedString(@"Quit", nil), applicationName]
								action:@selector(terminate:)
						 keyEquivalent:@"q"];
	[menuItem setTarget:NSApp];
}

+(void) populateDebugMenu:(NSMenu *)aMenu
{
	// TODO
}

+(void) populateEditMenu:(NSMenu *)aMenu
{
	NSMenuItem * menuItem;
	
	menuItem = [aMenu addItemWithTitle:NSLocalizedString(@"Undo", nil)
								action:@selector(undo:)
						 keyEquivalent:@"z"];
	
	menuItem = [aMenu addItemWithTitle:NSLocalizedString(@"Redo", nil)
								action:@selector(redo:)
						 keyEquivalent:@"Z"];
	
	[aMenu addItem:[NSMenuItem separatorItem]];
	
	menuItem = [aMenu addItemWithTitle:NSLocalizedString(@"Cut", nil)
								action:@selector(cut:)
						 keyEquivalent:@"x"];
	
	menuItem = [aMenu addItemWithTitle:NSLocalizedString(@"Copy", nil)
								action:@selector(copy:)
						 keyEquivalent:@"c"];
	
	menuItem = [aMenu addItemWithTitle:NSLocalizedString(@"Paste", nil)
								action:@selector(paste:)
						 keyEquivalent:@"v"];
	
	menuItem = [aMenu addItemWithTitle:NSLocalizedString(@"Paste and Match Style", nil)
								action:@selector(pasteAsPlainText:)
						 keyEquivalent:@"V"];
	[menuItem setKeyEquivalentModifierMask:NSCommandKeyMask | NSAlternateKeyMask];
	
	menuItem = [aMenu addItemWithTitle:NSLocalizedString(@"Delete", nil)
								action:@selector(delete:)
						 keyEquivalent:@""];
	
	menuItem = [aMenu addItemWithTitle:NSLocalizedString(@"Select All", nil)
								action:@selector(selectAll:)
						 keyEquivalent:@"a"];
	
	[aMenu addItem:[NSMenuItem separatorItem]];
	
	menuItem = [aMenu addItemWithTitle:NSLocalizedString(@"Find", nil)
								action:NULL
						 keyEquivalent:@""];
	NSMenu * findMenu = [[NSMenu alloc] initWithTitle:@"Find"];
	[self populateFindMenu:findMenu];
	[aMenu setSubmenu:findMenu forItem:menuItem];
	
	menuItem = [aMenu addItemWithTitle:NSLocalizedString(@"Spelling", nil)
								action:NULL
						 keyEquivalent:@""];
	NSMenu * spellingMenu = [[NSMenu alloc] initWithTitle:@"Spelling"];
	[self populateSpellingMenu:spellingMenu];
	[aMenu setSubmenu:spellingMenu forItem:menuItem];
}

+(void) populateFileMenu:(NSMenu *)aMenu
{
	NSMenuItem * menuItem;
	
	menuItem = [aMenu addItemWithTitle:NSLocalizedString(@"New", nil)
								action:NULL
						 keyEquivalent:@"n"];
	
	menuItem = [aMenu addItemWithTitle:NSLocalizedString(@"Open...", nil)
								action:NULL
						 keyEquivalent:@"o"];
	
	menuItem = [aMenu addItemWithTitle:NSLocalizedString(@"Open Recent", nil)
								action:NULL
						 keyEquivalent:@""];
	NSMenu * openRecentMenu = [[NSMenu alloc] initWithTitle:@"Open Recent"];
	[openRecentMenu performSelector:@selector(_setMenuName:) withObject:@"NSRecentDocumentsMenu"];
	[aMenu setSubmenu:openRecentMenu forItem:menuItem];
	
	menuItem = [openRecentMenu addItemWithTitle:NSLocalizedString(@"Clear Menu", nil)
										 action:@selector(clearRecentDocuments:)
								  keyEquivalent:@""];
	
	[aMenu addItem:[NSMenuItem separatorItem]];
	
	menuItem = [aMenu addItemWithTitle:NSLocalizedString(@"Close", nil)
								action:@selector(performClose:)
						 keyEquivalent:@"w"];
	
	menuItem = [aMenu addItemWithTitle:NSLocalizedString(@"Save", nil)
								action:NULL
						 keyEquivalent:@"s"];
	
	menuItem = [aMenu addItemWithTitle:NSLocalizedString(@"Save As...", nil)
								action:NULL
						 keyEquivalent:@"S"];
	
	menuItem = [aMenu addItemWithTitle:NSLocalizedString(@"Revert", nil)
								action:NULL
						 keyEquivalent:@""];
	
	[aMenu addItem:[NSMenuItem separatorItem]];
	
	menuItem = [aMenu addItemWithTitle:NSLocalizedString(@"Page Setup...", nil)
								action:@selector(runPageLayout:)
						 keyEquivalent:@"P"];
	
	menuItem = [aMenu addItemWithTitle:NSLocalizedString(@"Print...", nil)
								action:@selector(print:)
						 keyEquivalent:@"p"];
}

+(void) populateFindMenu:(NSMenu *)aMenu
{
	NSMenuItem * menuItem;
	
	menuItem = [aMenu addItemWithTitle:NSLocalizedString(@"Find...", nil)
								action:@selector(performFindPanelAction:)
						 keyEquivalent:@"f"];
	[menuItem setTag:NSFindPanelActionShowFindPanel];
	
	menuItem = [aMenu addItemWithTitle:NSLocalizedString(@"Find Next", nil)
								action:@selector(performFindPanelAction:)
						 keyEquivalent:@"g"];
	[menuItem setTag:NSFindPanelActionNext];
	
	menuItem = [aMenu addItemWithTitle:NSLocalizedString(@"Find Previous", nil)
								action:@selector(performFindPanelAction:)
						 keyEquivalent:@"G"];
	[menuItem setTag:NSFindPanelActionPrevious];
	
	menuItem = [aMenu addItemWithTitle:NSLocalizedString(@"Use Selection for Find", nil)
								action:@selector(performFindPanelAction:)
						 keyEquivalent:@"e"];
	[menuItem setTag:NSFindPanelActionSetFindString];
	
	menuItem = [aMenu addItemWithTitle:NSLocalizedString(@"Jump to Selection", nil)
								action:@selector(centerSelectionInVisibleArea:)
						 keyEquivalent:@"j"];
}

+(void) populateHelpMenu:(NSMenu *)aMenu
{
	NSMenuItem * menuItem;
	
	menuItem = [aMenu addItemWithTitle:[NSString stringWithFormat:@"%@ %@", @"root", NSLocalizedString(@"Help", nil)]
								action:@selector(showHelp:)
						 keyEquivalent:@"?"];
	[menuItem setTarget:NSApp];
}

+(void) populateSpellingMenu:(NSMenu *)aMenu
{
	NSMenuItem * menuItem;
	
	menuItem = [aMenu addItemWithTitle:NSLocalizedString(@"Spelling...", nil)
								action:@selector(showGuessPanel:)
						 keyEquivalent:@":"];
	
	menuItem = [aMenu addItemWithTitle:NSLocalizedString(@"Check Spelling", nil)
								action:@selector(checkSpelling:)
						 keyEquivalent:@";"];
	
	menuItem = [aMenu addItemWithTitle:NSLocalizedString(@"Check Spelling as You Type", nil)
								action:@selector(toggleContinuousSpellChecking:)
						 keyEquivalent:@""];
}

+(void) populateViewMenu:(NSMenu *)aMenu
{
	// TODO
}

+(void) populateWindowMenu:(NSMenu *)aMenu
{
	NSMenuItem * menuItem;
	
	menuItem = [aMenu addItemWithTitle:NSLocalizedString(@"Minimize", nil)
								action:@selector(performMinimize:)
						 keyEquivalent:@"m"];
	
	menuItem = [aMenu addItemWithTitle:NSLocalizedString(@"Zoom", nil)
								action:@selector(performZoom:)
						 keyEquivalent:@""];
	
	[aMenu addItem:[NSMenuItem separatorItem]];
	
	menuItem = [aMenu addItemWithTitle:NSLocalizedString(@"Bring All to Front", nil)
								action:@selector(arrangeInFront:)
						 keyEquivalent:@""];
}

@end

//
//Stuff which I have to copy from TUnixSystem. Find a better way to organize code.
//Fortunately, this does not violate ODR, but still UGLY.
//

//------------------- Unix TFdSet ----------------------------------------------
#ifndef HOWMANY
#   define HOWMANY(x, y)   (((x)+((y)-1))/(y))
#endif

const Int_t kNFDBITS = (sizeof(Long_t) * 8);  // 8 bits per byte
#ifdef FD_SETSIZE
const Int_t kFDSETSIZE = FD_SETSIZE;          // Linux = 1024 file descriptors
#else
const Int_t kFDSETSIZE = 256;                 // upto 256 file descriptors
#endif


class TFdSet {
private:
   ULong_t fds_bits[HOWMANY(kFDSETSIZE, kNFDBITS)];
public:
   TFdSet() { memset(fds_bits, 0, sizeof(fds_bits)); }
   TFdSet(const TFdSet &org) { memcpy(fds_bits, org.fds_bits, sizeof(org.fds_bits)); }
   TFdSet &operator=(const TFdSet &rhs) { if (this != &rhs) { memcpy(fds_bits, rhs.fds_bits, sizeof(rhs.fds_bits));} return *this; }
   void   Zero() { memset(fds_bits, 0, sizeof(fds_bits)); }
   void   Set(Int_t n)
   {
      if (n >= 0 && n < kFDSETSIZE) {
         fds_bits[n/kNFDBITS] |= (1UL << (n % kNFDBITS));
      } else {
         ::Fatal("TFdSet::Set","fd (%d) out of range [0..%d]", n, kFDSETSIZE-1);
      }
   }
   void   Clr(Int_t n)
   {
      if (n >= 0 && n < kFDSETSIZE) {
         fds_bits[n/kNFDBITS] &= ~(1UL << (n % kNFDBITS));
      } else {
         ::Fatal("TFdSet::Clr","fd (%d) out of range [0..%d]", n, kFDSETSIZE-1);
      }
   }
   Int_t  IsSet(Int_t n)
   {
      if (n >= 0 && n < kFDSETSIZE) {
         return (fds_bits[n/kNFDBITS] & (1UL << (n % kNFDBITS))) != 0;
      } else {
         ::Fatal("TFdSet::IsSet","fd (%d) out of range [0..%d]", n, kFDSETSIZE-1);
         return 0;
      }
   }
   ULong_t *GetBits() { return (ULong_t *)fds_bits; }
};

namespace ROOT {
namespace MacOSX {
namespace Detail {

class MacOSXSystem {
public:
   MacOSXSystem();
   ~MacOSXSystem();


   void AddFileHandler(TFileHandler *fh);
   void RemoveFileHandler(TFileHandler *fh);

   bool SetFileDescriptors();
   void UnregisterFileDescriptor(CFFileDescriptorRef fd);
   void CloseFileDescriptors();

   enum DescriptorType {
      kDTWrite,
      kDTRead
   };

   //Before I had C++11 and auto, now I have ugly typedefs.
   typedef std::map<int, unsigned> fd_map_type;
   typedef fd_map_type::iterator fd_map_iterator;
   typedef fd_map_type::const_iterator const_fd_map_iterator;
   
   fd_map_type fReadFds;
   fd_map_type fWriteFds;
   
   static void RemoveFileDescriptor(fd_map_type &fdTable, int fd);
   void SetFileDescriptors(const fd_map_type &fdTable, DescriptorType fdType);

   std::set<CFFileDescriptorRef> fCFFileDescriptors;
   const ROOT::MacOSX::Util::AutoreleasePool fPool;

   static MacOSXSystem *fgInstance;
};

MacOSXSystem *MacOSXSystem::fgInstance = nullptr;

extern "C" {

//______________________________________________________________________________
void TMacOSXSystem_ReadCallback(CFFileDescriptorRef fdref, CFOptionFlags /*callBackTypes*/, void * /*info*/)
{
   //Native descriptor.
   const int nativeFD = CFFileDescriptorGetNativeDescriptor(fdref);

   //We do not need this descriptor anymore.
   assert(MacOSXSystem::fgInstance != nullptr && "TMacOSXSystem_ReadCallback, MacOSXSystem's singleton is null");   
   MacOSXSystem::fgInstance->UnregisterFileDescriptor(fdref);
   
   CFFileDescriptorInvalidate(fdref);
   CFRelease(fdref);

   NSEvent *fdEvent = [NSEvent otherEventWithType : NSApplicationDefined location : NSMakePoint(0, 0) modifierFlags : 0
                       timestamp: 0. windowNumber : 0 context : nil subtype : 0 data1 : nativeFD data2 : 0];
   [NSApp postEvent : fdEvent atStart : NO];
}

//______________________________________________________________________________
void TMacOSXSystem_WriteCallback(CFFileDescriptorRef fdref, CFOptionFlags /*callBackTypes*/, void * /*info*/)
{
   //Native descriptor.
   const int nativeFD = CFFileDescriptorGetNativeDescriptor(fdref);

   //We do not need this descriptor anymore.
   assert(MacOSXSystem::fgInstance != nullptr && "TMacOSXSystem_WriteCallback, MacOSXSystem's singleton is null");   
   MacOSXSystem::fgInstance->UnregisterFileDescriptor(fdref);

   CFFileDescriptorInvalidate(fdref);
   CFRelease(fdref);

   NSEvent *fdEvent = [NSEvent otherEventWithType : NSApplicationDefined location : NSMakePoint(0, 0) modifierFlags : 0
                       timestamp: 0. windowNumber : 0 context : nil subtype : 0 data1 : nativeFD data2 : 0];
   [NSApp postEvent : fdEvent atStart : NO];
}

}

//______________________________________________________________________________
MacOSXSystem::MacOSXSystem()
{
   assert(fgInstance == 0 && "MacOSXSystem, fgInstance was initialized already");

   fgInstance = this;
}

//______________________________________________________________________________
MacOSXSystem::~MacOSXSystem()
{
   CloseFileDescriptors();
}

//______________________________________________________________________________
void MacOSXSystem::AddFileHandler(TFileHandler *fh)
{
   //Can throw std::bad_alloc. I'm not allocating any resources here.
   assert(fh != 0 && "AddFileHandler, fh parameter is null");
   
   if (fh->HasReadInterest())
      fReadFds[fh->GetFd()]++;
   
   //Can we have "duplex" fds?

   if (fh->HasWriteInterest())
      fWriteFds[fh->GetFd()]++;
}

//______________________________________________________________________________
void MacOSXSystem::RemoveFileHandler(TFileHandler *fh)
{
   //Can not throw.

   //ROOT has obvious bugs somewhere: the same fd can be removed MORE times,
   //than it was added.

   assert(fh != 0 && "RemoveFileHandler, fh parameter is null");

   if (fh->HasReadInterest())
      RemoveFileDescriptor(fReadFds, fh->GetFd());
   
   if (fh->HasWriteInterest())
      RemoveFileDescriptor(fWriteFds, fh->GetFd());
}

//______________________________________________________________________________
bool MacOSXSystem::SetFileDescriptors()
{
   //Allocates some resources and can throw.
   //So, make sure resources are freed correctly
   //in case of exception (std::bad_alloc) and
   //return false. Return true if everything is ok.

   try {
      if (fReadFds.size())
         SetFileDescriptors(fReadFds, kDTRead);
      if (fWriteFds.size())
         SetFileDescriptors(fWriteFds, kDTWrite);
   } catch (const std::exception &) {
      CloseFileDescriptors();
      return false;
   }

   return true;
}

//______________________________________________________________________________
void MacOSXSystem::UnregisterFileDescriptor(CFFileDescriptorRef fd)
{
   //This function does not touch file descriptor (it's invalidated and released externally),
   //just remove pointer.

   std::set<CFFileDescriptorRef>::iterator fdIter = fCFFileDescriptors.find(fd);
   assert(fdIter != fCFFileDescriptors.end() && "InvalidateFileDescriptor, file descriptor was not found");
   fCFFileDescriptors.erase(fdIter);
}

//______________________________________________________________________________
void MacOSXSystem::CloseFileDescriptors()
{
   for (std::set<CFFileDescriptorRef>::iterator fdIter = fCFFileDescriptors.begin(), end = fCFFileDescriptors.end(); fdIter != end; ++fdIter) {
      CFFileDescriptorInvalidate(*fdIter);
      CFRelease(*fdIter);
   }   

   fCFFileDescriptors.clear();
}

//______________________________________________________________________________
void MacOSXSystem::RemoveFileDescriptor(fd_map_type &fdTable, int fd)
{
   fd_map_iterator fdIter = fdTable.find(fd);

   if (fdIter != fdTable   .end()) {
      assert(fdIter->second != 0 && "RemoveFD, 'dead' descriptor in a table");
      if (!(fdIter->second - 1))
         fdTable.erase(fdIter);
      else
         --fdIter->second;
   } else {
      //I had to comment warning, many thanks to ROOT for this bizarre thing.
      //::Warning("RemoveFileDescriptor", "Descriptor %d was not found in a table", fd);
   }
}

//______________________________________________________________________________
void MacOSXSystem::SetFileDescriptors(const fd_map_type &fdTable, DescriptorType fdType)
{
   for (const_fd_map_iterator fdIter = fdTable.begin(), end = fdTable.end(); fdIter != end; ++fdIter) {
      const bool read = fdType == kDTRead;
      CFFileDescriptorRef fdref = CFFileDescriptorCreate(kCFAllocatorDefault, fdIter->first, false, read ? TMacOSXSystem_ReadCallback : TMacOSXSystem_WriteCallback, 0);

      if (!fdref)
         throw std::runtime_error("MacOSXSystem::SetFileDescriptors: CFFileDescriptorCreate failed");

      CFFileDescriptorEnableCallBacks(fdref, read ? kCFFileDescriptorReadCallBack : kCFFileDescriptorWriteCallBack);
   
      CFRunLoopSourceRef runLoopSource = CFFileDescriptorCreateRunLoopSource(kCFAllocatorDefault, fdref, 0);
      
      if (!runLoopSource) {
         CFRelease(fdref);
         throw std::runtime_error("MacOSXSystem::SetFileDescriptors: CFFileDescriptorCreateRunLoopSource failed");
      }

      CFRunLoopAddSource(CFRunLoopGetMain(), runLoopSource, kCFRunLoopDefaultMode);
      CFRelease(runLoopSource);

      fCFFileDescriptors.insert(fdref);
   }
}

}//Detail
}//MacOSX
}//ROOT

namespace Private = ROOT::MacOSX::Detail;

ClassImp(TMacOSXSystem)

//______________________________________________________________________________
TMacOSXSystem::TMacOSXSystem()
                  : fPimpl(new Private::MacOSXSystem),
                    fCallAppRun(true)
{
   [NSApplication sharedApplication];

   //Documentation says, that +sharedApplication, initializes the app. But this is not true,
   //it's still not really initialized, part of initialization is done by -run method.

   //If you call run, it never returns unless app is finished. I have to stop Cocoa's event loop
   //processing, since we have our own event loop.
   
   const ROOT::MacOSX::Util::NSScopeGuard<RunStopper> stopper([[RunStopper alloc] init]);

   [stopper.Get() performSelector : @selector(stopRun) withObject : nil afterDelay : 0.05];//Delay? What it should be?
   [NSApp run];
}

//______________________________________________________________________________
TMacOSXSystem::~TMacOSXSystem()
{
}

//______________________________________________________________________________
void TMacOSXSystem::ProcessApplicationDefinedEvent(void *e)
{
   //Right now I have app. defined events only
   //for file descriptors. This can change in a future.
   NSEvent *event = (NSEvent *)e;

   assert(event != nil && "ProcessApplicationDefinedEvent, event parameter is nil");
   assert(event.type == NSApplicationDefined && "ProcessApplicationDefinedEvent, event parameter has wrong type");
   
   Private::MacOSXSystem::fd_map_iterator fdIter = fPimpl->fReadFds.find(event.data1);
   bool descriptorFound = false;

   if (fdIter != fPimpl->fReadFds.end()) {
      fReadready->Set(event.data1);
      descriptorFound = true;
   }
   
   fdIter = fPimpl->fWriteFds.find(event.data1);
   if (fdIter != fPimpl->fWriteFds.end()) {
      fWriteready->Set(event.data1);
      descriptorFound = true;
   }
   
   if (!descriptorFound) {
      Error("ProcessApplicationDefinedEvent", "file descriptor %d was not found", int(event.data1));
      return;
   }
   
   ++fNfd;
}

//______________________________________________________________________________
void TMacOSXSystem::WaitEvents(Long_t nextto)
{
   //Wait for GUI/Non-GUI events.

   if (!fPimpl->SetFileDescriptors()) {
      //I consider this error as fatal.
      Fatal("WaitForAllEvents", "SetFileDesciptors failed");
   }

   NSDate *untilDate = nil;
   if (nextto >= 0)//0 also means non-blocking call.
      untilDate = [NSDate dateWithTimeIntervalSinceNow : nextto / 1000.];
   else
      untilDate = [NSDate distantFuture];

   fReadready->Zero();
   fWriteready->Zero();
   fNfd = 0;
   
   bool hadGUIEvent = false;

   NSEvent *event = [NSApp nextEventMatchingMask : NSAnyEventMask untilDate : untilDate inMode : NSDefaultRunLoopMode dequeue : YES];

   if (event.type == NSApplicationDefined)
      ProcessApplicationDefinedEvent(event);
   else {
      hadGUIEvent = true;
      [NSApp sendEvent : event];
   }

   while ((event = [NSApp nextEventMatchingMask : NSAnyEventMask untilDate : nil inMode : NSDefaultRunLoopMode dequeue : YES])) {
      if (event.type == NSApplicationDefined)
         ProcessApplicationDefinedEvent(event);
      else {
         hadGUIEvent = true;
         [NSApp sendEvent : event];
      }
   }

   fPimpl->CloseFileDescriptors();

   gVirtualX->Update(2);
   gVirtualX->Update(3);
   
   if (fCallAppRun && hadGUIEvent) {
      fCallAppRun = false;
      /*NSEvent* stopEvent = [NSEvent otherEventWithType: NSApplicationDefined
                                     location: NSMakePoint(0,0)
                               modifierFlags: 0
                                   timestamp: 0.0
                                 windowNumber: 0
                                     context: nil
                                     subtype: 0
                                       data1: 0
                                       data2: 0];
      [NSApp postEvent : stopEvent atStart: true];
      [NSApp stop:NSApp];//performSelector : @selector(stop:) withObject : nil afterDelay : 0.05];

      [NSApp run];*/
      [MenuLoader populateMainMenu];
   }
}

//______________________________________________________________________________
void TMacOSXSystem::DispatchOneEvent(Bool_t pendingOnly)
{
   //Here I try to emulate TUnixSystem's behavior, which is quite twisted.
   //I'm not even sure, I need all this code :)   
   Bool_t pollOnce = pendingOnly;

   while (true) {
      const ROOT::MacOSX::Util::AutoreleasePool pool;

      //Check for file descriptors ready for reading/writing.
      if (fNfd > 0 && fFileHandler && fFileHandler->GetSize() > 0)
         if (CheckDescriptors())
            if (!pendingOnly)
               return;

      fNfd = 0;
      fReadready->Zero();
      fWriteready->Zero();

      if (pendingOnly && !pollOnce)
         return;

      // check synchronous signals
      if (fSigcnt > 0 && fSignalHandler->GetSize() > 0)
         if (CheckSignals(kTRUE))
            if (!pendingOnly) return;

      fSigcnt = 0;
      fSignals->Zero();

      // check synchronous timers
      Long_t nextto = 0;
      if (fTimers && fTimers->GetSize() > 0) {
         if (DispatchTimers(kTRUE)) {
            // prevent timers from blocking file descriptor monitoring
            nextto = NextTimeOut(kTRUE);
            if (nextto > kItimerResolution || nextto == -1)
               return;
         }
      }

      // if in pendingOnly mode poll once file descriptor activity
      nextto = NextTimeOut(kTRUE);
      
      if (pendingOnly) {
         if (fFileHandler && !fFileHandler->GetSize())
            return;
         nextto = 0;
         pollOnce = kFALSE;
      }

      //Wait for GUI events and for something else, like read/write from stdin/stdout (?).
      WaitEvents(nextto);
      
      if (gXDisplay && gXDisplay->Notify()) {
         gVirtualX->Update(2);
         gVirtualX->Update(3);
         if (!pendingOnly) 
            return;
      }      

      if (pendingOnly)
         return;
   }
}

//______________________________________________________________________________
void TMacOSXSystem::AddFileHandler(TFileHandler *fh)
{
   fPimpl->AddFileHandler(fh);
   TUnixSystem::AddFileHandler(fh);
}

//______________________________________________________________________________
TFileHandler *TMacOSXSystem::RemoveFileHandler(TFileHandler *fh)
{
   fPimpl->RemoveFileHandler(fh);
   return TUnixSystem::RemoveFileHandler(fh);
}
