//#define DEBUG_ROOT_COCOA

#include <stdexcept>
#include <vector>
#include <map>

#import <Cocoa/Cocoa.h>

#include "TSeqCollection.h"
#include "TMacOSXSystem.h"
#include "CocoaUtils.h"
#include "TVirtualX.h"
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

//______________________________________________________________________________
void TMacOSXSystem_ReadCallback(CFFileDescriptorRef fdref, CFOptionFlags /*callBackTypes*/, void * /*info*/)
{
   //We do not need this descriptor anymore.
   CFFileDescriptorInvalidate(fdref);
   CFRelease(fdref);
   
   NSEvent *fdEvent = [NSEvent otherEventWithType : NSApplicationDefined location : NSMakePoint(0, 0) modifierFlags : 0
                       timestamp: 0. windowNumber : 0 context : nil subtype : 0 data1 : 0 data2 : 0];
   [NSApp postEvent : fdEvent atStart : NO];
}

//______________________________________________________________________________
void TMacOSXSystem_WriteCallback(CFFileDescriptorRef fdref, CFOptionFlags /*callBackTypes*/, void * /*info*/)
{
   //We do not need this descriptor anymore.
   CFFileDescriptorInvalidate(fdref);
   CFRelease(fdref);

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
   
   std::vector<CFFileDescriptorRef> fCFFileDescriptors;

   bool AddFileHandler(TFileHandler *fh);
   bool RemoveFileHandler(TFileHandler *fh);
   
   bool SetFileDescriptors();
   void CloseFileDescriptors();
   void CleanDescriptorArrays();
   
   const ROOT::MacOSX::Util::AutoreleasePool fPool;
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

   try {
      for (auto fdIter = fFileDescriptors.begin(), end = fFileDescriptors.end(); fdIter != end; ++fdIter) {
         const bool read = fdIter->second == DescriptorType::read;
         CFFileDescriptorRef fdref = CFFileDescriptorCreate(kCFAllocatorDefault, fdIter->first, false, read ? TMacOSXSystem_ReadCallback : TMacOSXSystem_WriteCallback, 0);

         if (!fdref)
            throw std::runtime_error("TMacOSXSystemPrivate::SetFileDescriptors: CFFileDescriptorCreate failed");

         CFFileDescriptorEnableCallBacks(fdref, read ? kCFFileDescriptorReadCallBack : kCFFileDescriptorWriteCallBack);
      
         CFRunLoopSourceRef runLoopSource = CFFileDescriptorCreateRunLoopSource(kCFAllocatorDefault, fdref, 0);
         
         if (!runLoopSource) {
            CFRelease(fdref);
            throw std::runtime_error("TMacOSXSystemPrivate::SetFileDescriptors: CFFileDescriptorCreateRunLoopSource failed");
         }

         CFRunLoopAddSource(CFRunLoopGetMain(), runLoopSource, kCFRunLoopDefaultMode);
         CFRelease(runLoopSource);

         fCFFileDescriptors.push_back(fdref);
      }
   } catch (const std::exception &e) {
      CloseFileDescriptors();
      return false;
   }

   return true;
}

//______________________________________________________________________________
void TMacOSXSystemPrivate::CloseFileDescriptors()
{
   
   for (auto fd : fCFFileDescriptors) {
      CFFileDescriptorInvalidate(fd);
      CFRelease(fd);
   }   
   
   CleanDescriptorArrays();
}

//______________________________________________________________________________
void TMacOSXSystemPrivate::CleanDescriptorArrays()
{
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
/*   bool hadGuiEvent = false;

#ifdef DEBUG_ROOT_COCOA
   NSLog(@"Enter non-blocking loop");
#endif

   while (true) {
      NSEvent *event = [NSApp nextEventMatchingMask : NSAnyEventMask untilDate : nil inMode : NSDefaultRunLoopMode dequeue : YES];
      if (event) {
         hadGuiEvent = true;
         [NSApp sendEvent : event];
         //Process event.
#ifdef DEBUG_ROOT_COCOA
         NSLog(@"Non blocking processing: got event %@", event);
#endif
      } else {
         break;
      }
   }

#ifdef DEBUG_ROOT_COCOA
   NSLog(@"Exit non-blocking loop");
#endif

   return hadGuiEvent;*/
   return false;
}

//______________________________________________________________________________
void TMacOSXSystem::WaitForGuiEvents(Long_t nextto)
{
   //Wait for one event, do not dequeue (will be done by the following non-blocking call).
#ifdef DEBUG_ROOT_COCOA
   NSLog(@"blocking call now - WaitForGuiEvents");
#endif

   NSDate *untilDate = nil;
   if (nextto >= 0)//0 also means non-blocking call.
      untilDate = [NSDate dateWithTimeIntervalSinceNow : nextto / 1000.];
   else
      untilDate = [NSDate distantFuture];

   NSEvent *event = [NSApp nextEventMatchingMask : NSAnyEventMask untilDate : untilDate inMode : NSDefaultRunLoopMode dequeue : YES];
   //[NSApp postEvent : event atStart : YES];
   [NSApp sendEvent : event];
   //
   gVirtualX->Update(1);
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

   NSDate *untilDate = nil;
   if (nextto >= 0)//0 also means non-blocking call.
      untilDate = [NSDate dateWithTimeIntervalSinceNow : nextto / 1000.];
   else
      untilDate = [NSDate distantFuture];

   NSEvent *event = [NSApp nextEventMatchingMask : NSAnyEventMask untilDate : untilDate inMode : NSDefaultRunLoopMode dequeue : YES];

   if (event.type == NSApplicationDefined) {
      //TODO: this check is only for test, do it right later (somehow identify an event in a CFFileDescriptor).
      //Remove from event queue.
#ifdef DEBUG_ROOT_COCOA
      NSLog(@"got app defined event, try to remove from the queue");
#endif

      // nothing ready, so setup select call
      *fReadready  = *fReadmask;
      *fWriteready = *fWritemask;
      fNfd = 1;
      
      fPimpl->CleanDescriptorArrays();
   } else {
#ifdef DEBUG_ROOT_COCOA
      NSLog(@"got a GUI (?) event %@", event);
#endif
      //[NSApp postEvent : event atStart : YES];
      [NSApp sendEvent : event];
      //
      fPimpl->CloseFileDescriptors();
   }

   gVirtualX->Update(1);
}

//______________________________________________________________________________
void TMacOSXSystem::DispatchOneEvent(Bool_t pendingOnly)
{
   //Here I try to emulate TUnixSystem's behavior, which is quite twisted.

   
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

      //Now, do a trick.
      if (!fPimpl->fFileDescriptors.size()) {
         //ConnectionNumber(x11display) is a file descriptor,
         //but for Cocoa I do not have such a thing. Make a blocking call here, like I have a connection number and POSIX select.
         WaitForGuiEvents(nextto);
      } else {
         //Wait for GUI events and for something else, like read/write from stdin/stdout (?).
         WaitForAllEvents(nextto);
         //???
         if (pendingOnly)
            return;
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
