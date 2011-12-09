//#define DEBUG_ROOT_COCOA

#include <stdexcept>
#include <vector>
#include <map>

#import <Cocoa/Cocoa.h>

#include "TSeqCollection.h"
#include "TMacOSXSystem.h"
#include "CocoaUtils.h"
#include "TError.h"

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

//
//Callbacks for file descriptors.
//

extern "C" {

void TMacOSXSystem_ReadCallback(CFFileDescriptorRef /*fdref*/, CFOptionFlags /*callBackTypes*/, void * /*info*/)
{
#ifdef DEBUG_ROOT_COCOA
   NSLog(@"ReadCallback was called, posting event");
#endif
   //
   NSEvent *fdEvent = [NSEvent otherEventWithType : NSApplicationDefined location : NSMakePoint(0, 0) modifierFlags : 0
                       timestamp: 0. windowNumber : 0 context : nil subtype : 0 data1 : 0 data2 : 0];
   [NSApp postEvent : fdEvent atStart : NO];
}

void TMacOSXSystem_WriteCallback(CFFileDescriptorRef /*fdref*/, CFOptionFlags /*callBackTypes*/, void * /*info*/)
{
#ifdef DEBUG_ROOT_COCOA
   NSLog(@"WriteCallback was called, posting event");
#endif
   NSEvent *fdEvent = [NSEvent otherEventWithType : NSApplicationDefined location : NSMakePoint(0, 0) modifierFlags : 0
                       timestamp: 0. windowNumber : 0 context : nil subtype : 0 data1 : 0 data2 : 0];
   [NSApp postEvent : fdEvent atStart : NO];
}

}

//
//Hidden implementation details (in particular, hidden from CINT).
//

class TMacOSXSystemPrivate {
   enum class DescriptorType {
      write,
      read
   };

#ifdef DEBUG_ROOT_COCOA
public:
   ~TMacOSXSystemPrivate();
private:
#endif

   friend class TMacOSXSystem;
   std::map<int, DescriptorType> fFileDescriptors;
   
   typedef ROOT::MacOSX::Util::CFGuard<CFFileDescriptorRef> cffile_type;
   std::vector<cffile_type> fCFFileDescriptors;

   typedef ROOT::MacOSX::Util::CFGuard<CFRunLoopSourceRef> cfrl_type;
   std::vector<cfrl_type> fRunLoopSources;
   
   bool AddFileHandler(TFileHandler *fh);
   bool RemoveFileHandler(TFileHandler *fh);
   
   bool SetFileDescriptors();
   void CloseFileDescriptors();
};

#ifdef DEBUG_ROOT_COCOA
//______________________________________________________________________________
TMacOSXSystemPrivate::~TMacOSXSystemPrivate()
{
   if (fCFFileDescriptors.size()) {
      NSLog(@"TMacOSXSystemPrivate::~TMacOSXSystemPrivate, file descriptors were not closed!!!");
      CloseFileDescriptors();
   }
}
#endif

//______________________________________________________________________________
bool TMacOSXSystemPrivate::AddFileHandler(TFileHandler *fh)
{
   //Can throw std::bad_alloc. I'm not allocating any resources here, so I'm not going to catch here.

#ifdef DEBUG_ROOT_COCOA
   NSLog(@"TMacOSXCSystemPrivate::AddFileHandler: fd is %d", fh->GetFd());
#endif

   if (fFileDescriptors.find(fh->GetFd()) == fFileDescriptors.end()) {
      fFileDescriptors[fh->GetFd()] = fh->HasReadInterest() ? DescriptorType::read : DescriptorType::write;
   } else {
#ifdef DEBUG_ROOT_COCOA
      NSLog(@"TMacOSXSystemPrivate::AddFileHandler: file descriptor %d was registered already", fh->GetFd());
#endif
      return false;
   }

   return true;
}

//______________________________________________________________________________
bool TMacOSXSystemPrivate::RemoveFileHandler(TFileHandler *fh)
{
   //Can not throw.

#ifdef DEBUG_ROOT_COCOA
   NSLog(@"TMacOSXSystemPrivate::RemoveFileHandler, file descriptor is %d", fh->GetFd());
#endif

   auto fdIter = fFileDescriptors.find(fh->GetFd());
   if (fdIter != fFileDescriptors.end()) {
      fFileDescriptors.erase(fdIter);
   } else {
#ifdef DEBUG_ROOT_COCOA
      NSLog(@"Attempt to remove unregistered file handler!");
#endif
      return false;
   }

   return true;
}

//______________________________________________________________________________
bool TMacOSXSystemPrivate::SetFileDescriptors()
{
   //Allocates some resources and can throw.
   //So, make sure resources are freed correctly
   //in case of exception (std::bad_alloc) and
   //return false. Return true if everything is ok.

#ifdef DEBUG_ROOT_COCOA
   NSLog(@"---------SetFileDescriptors");
#endif

   try {
      for (auto fdIter = fFileDescriptors.begin(), end = fFileDescriptors.end(); fdIter != end; ++fdIter) {
         const bool read = fdIter->second == DescriptorType::read;
         cffile_type fdref(CFFileDescriptorCreate(kCFAllocatorDefault, fdIter->first, false, read ? TMacOSXSystem_ReadCallback : TMacOSXSystem_WriteCallback, 0), false);//Check how to use red/write callbacks or one callback later.

         if (!fdref.Get())
            throw std::runtime_error("TMacOSXSystemPrivate::SetFileDescriptors: CFFileDescriptorCreate failed");

         CFFileDescriptorEnableCallBacks(fdref.Get(), read ? kCFFileDescriptorReadCallBack : kCFFileDescriptorWriteCallBack);
      
         cfrl_type runLoopSource(CFFileDescriptorCreateRunLoopSource(kCFAllocatorDefault, fdref.Get(), 0), false);
         if (!runLoopSource.Get())
            throw std::runtime_error("TMacOSXSystemPrivate::SetFileDescriptors: CFFileDescriptorCreateRunLoopSource failed");
   #ifdef DEBUG_ROOT_COCOA
         NSLog(@"Set file descriptor for %d", fdIter->first);
   #endif
         CFRunLoopAddSource(CFRunLoopGetMain(), runLoopSource.Get(), kCFRunLoopDefaultMode);

         fRunLoopSources.push_back(runLoopSource);      
         fCFFileDescriptors.push_back(fdref);
      }
   } catch (const std::exception &e) {
#ifdef DEBUG_ROOT_COCOA
      NSLog(@"TMacOSXSystemPrivate::SetFileDescriptors: %s", e.what());
#endif
      for (auto runLoop : fRunLoopSources)
         CFRunLoopRemoveSource(CFRunLoopGetMain(), runLoop.Get(), kCFRunLoopDefaultMode);
      fRunLoopSources.clear();
      fCFFileDescriptors.clear();
      
      return false;
   }

#ifdef DEBUG_ROOT_COCOA
   NSLog(@"SetFileDescriptors----------");
#endif

   return true;
}

//______________________________________________________________________________
void TMacOSXSystemPrivate::CloseFileDescriptors()
{
   for(auto runLoop : fRunLoopSources)
      CFRunLoopRemoveSource(CFRunLoopGetMain(), runLoop.Get(), kCFRunLoopDefaultMode);

   fRunLoopSources.clear();
   fCFFileDescriptors.clear();
}


ClassImp(TMacOSXSystem)

//______________________________________________________________________________
TMacOSXSystem::TMacOSXSystem()
                  : fPimpl(new TMacOSXSystemPrivate)
{

#ifdef DEBUG_ROOT_COCOA
   NSLog(@"TMacOSXSystem::TMacOSXSystem");
   NSLog(@"Init NSApplication ... ");
#endif

   [NSApplication sharedApplication];
   
#ifdef DEBUG_ROOT_COCOA
   NSLog(@"NSApplication is %@", [NSApplication sharedApplication]);
#endif
}

//______________________________________________________________________________
TMacOSXSystem::~TMacOSXSystem()
{
}

//______________________________________________________________________________
bool TMacOSXSystem::ProcessGuiEvents()
{
   //This is a non-blocking function ('untilDate' is nil, so we try to take previous events (?), not waiting).
   bool hadGuiEvent = false;

#ifdef DEBUG_ROOT_COCOA
   NSLog(@"Enter non-blocking loop");
#endif

   while (true) {
      NSEvent *event = [NSApp nextEventMatchingMask : NSAnyEventMask untilDate : nil inMode : NSDefaultRunLoopMode dequeue : YES];
      if (event) {
         hadGuiEvent = true;
         //Process event.
#ifdef DEBUG_ROOT_COCOA
         NSLog(@"Non blocking processing: got event %@", event);
#endif
      } else
         break;
   }

#ifdef DEBUG_ROOT_COCOA
   NSLog(@"Exit non-blocking loop");
#endif

   return hadGuiEvent;
}

//______________________________________________________________________________
void TMacOSXSystem::WaitForGuiEvents()
{
   //Wait for one event, do not dequeue (will be done by the following non-blocking call).
#ifdef DEBUG_ROOT_COCOA
   NSLog(@"blocking call now - WaitForGuiEvents");
#endif

   /*NSEvent *event = */[NSApp nextEventMatchingMask : NSAnyEventMask untilDate : [NSDate distantFuture] inMode : NSDefaultRunLoopMode dequeue : NO];
}

//______________________________________________________________________________
void TMacOSXSystem::WaitForAllEvents(Long_t nextto)
{
   //Wait for one event, do not dequeue (will be done by the following non-blocking call).
#ifdef DEBUG_ROOT_COCOA
   NSLog(@"blocking call now - WaitForAllEvents");
#endif

   if (!fPimpl->SetFileDescriptors()) {
      //I consider this error as fatal.
      Fatal("WaitForAllEvents", "SetFileDesciptors failed");
   }

   NSEvent *event = [NSApp nextEventMatchingMask : NSAnyEventMask untilDate : [NSDate distantFuture] inMode : NSDefaultRunLoopMode dequeue : YES];

   if (event.type == NSApplicationDefined) {
      //TODO: this check is only for test, do it right later (somehow identify an event in a CFFileDescriptor).
      //Remove from event queue.
#ifdef DEBUG_ROOT_COCOA
      NSLog(@"got app defined event, try to remove from the queue");
#endif

      int mxfd = TMath::Max(fMaxrfd, fMaxwfd);
      mxfd++;
      // nothing ready, so setup select call
      *fReadready  = *fReadmask;
      *fWriteready = *fWritemask;
      fNfd = 1;
   } else {
#ifdef DEBUG_ROOT_COCOA
      NSLog(@"got a GUI (?) event %@", event);
#endif
      //[NSApp postEvent : event atStart : YES];
      //Process GUI event HERE, DO NOT post it again (this will lead later to failure in CFFileDescriptorCreate for stdin (why????).
   }
   
   fPimpl->CloseFileDescriptors();
}

//______________________________________________________________________________
void TMacOSXSystem::DispatchOneEvent(Bool_t pendingOnly)
{
   //Here I try to emulate TUnixSystem's behavior, which is quite twisted.
   
   Bool_t pollOnce = pendingOnly;

   while (true) {
      //First handle any GUI events. Non-blocking call.
      if (gXDisplay && ProcessGuiEvents()) {
         if (pendingOnly)
            return;
      }

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

      //Now, do a trick.
      if (!fPimpl->fFileDescriptors.size()) {
         //ConnectionNumber(x11display) is a file descriptor,
         //but for Cocoa I do not have such a thing. Make a blocking call here, like I have a connection number and POSIX select.
         WaitForGuiEvents();
      } else {
         //Wait for GUI events and for something else, like read/write from stdin/stdout (?).
         WaitForAllEvents(nextto);
      }
   }
}

//______________________________________________________________________________
void TMacOSXSystem::AddFileHandler(TFileHandler *fh)
{
   if (fPimpl->AddFileHandler(fh))
      TUnixSystem::AddFileHandler(fh);
}

//______________________________________________________________________________
TFileHandler *TMacOSXSystem::RemoveFileHandler(TFileHandler *fh)
{
   if (fPimpl->RemoveFileHandler(fh))
      return TUnixSystem::RemoveFileHandler(fh);
   return fh;
}
