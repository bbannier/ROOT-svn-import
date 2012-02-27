//Author: Timur Pocheptsov 5/12/2011

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
   //Native descriptor.
   const int nativeFD = CFFileDescriptorGetNativeDescriptor(fdref);

   //We do not need this descriptor anymore.
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
   CFFileDescriptorInvalidate(fdref);
   CFRelease(fdref);

   NSEvent *fdEvent = [NSEvent otherEventWithType : NSApplicationDefined location : NSMakePoint(0, 0) modifierFlags : 0
                       timestamp: 0. windowNumber : 0 context : nil subtype : 0 data1 : nativeFD data2 : 0];
   [NSApp postEvent : fdEvent atStart : NO];
}

}

namespace ROOT {
namespace MacOSX {
namespace Detail {

class MacOSXSystem {
private:
   enum class DescriptorType {
      write,
      read
   };
#ifdef DEBUG_ROOT_COCOA
public:
   ~TMacOSXSystemPrivate();
private:
#endif

   friend class ::TMacOSXSystem;
   std::map<int, DescriptorType> fFileDescriptors;
   
   std::vector<CFFileDescriptorRef> fCFFileDescriptors;

   void AddFileHandler(TFileHandler *fh);
   void RemoveFileHandler(TFileHandler *fh);
   
   bool SetFileDescriptors();
   void CloseFileDescriptors();
   void CleanDescriptorArrays();
   
   const ROOT::MacOSX::Util::AutoreleasePool fPool;
};

#ifdef DEBUG_ROOT_COCOA

//______________________________________________________________________________
MacOSXSystem::~MacOSXSystem()
{
   if (fCFFileDescriptors.size()) {
      NSLog(@"TMacOSXSystemPrivate::~TMacOSXSystemPrivate, file descriptors were not closed!!!");
      CloseFileDescriptors();
   }
}

#endif

//______________________________________________________________________________
void MacOSXSystem::AddFileHandler(TFileHandler *fh)
{
   //Can throw std::bad_alloc. I'm not allocating any resources here, so I'm not going to catch here.
   assert(fFileDescriptors.find(fh->GetFd()) == fFileDescriptors.end() && "AddFileHandler, file descriptor was registered already");

   fFileDescriptors[fh->GetFd()] = fh->HasReadInterest() ? DescriptorType::read : DescriptorType::write;
}

//______________________________________________________________________________
void MacOSXSystem::RemoveFileHandler(TFileHandler *fh)
{
   //Can not throw.

   auto fdIter = fFileDescriptors.find(fh->GetFd());
   assert(fdIter != fFileDescriptors.end() && "RemoveFileHandler, file handler was not found");
   fFileDescriptors.erase(fdIter);
}

//______________________________________________________________________________
bool MacOSXSystem::SetFileDescriptors()
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
            throw std::runtime_error("MacOSXSystem::SetFileDescriptors: CFFileDescriptorCreate failed");

         CFFileDescriptorEnableCallBacks(fdref, read ? kCFFileDescriptorReadCallBack : kCFFileDescriptorWriteCallBack);
      
         CFRunLoopSourceRef runLoopSource = CFFileDescriptorCreateRunLoopSource(kCFAllocatorDefault, fdref, 0);
         
         if (!runLoopSource) {
            CFRelease(fdref);
            throw std::runtime_error("MacOSXSystem::SetFileDescriptors: CFFileDescriptorCreateRunLoopSource failed");
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
void MacOSXSystem::CloseFileDescriptors()
{
   
   for (auto fd : fCFFileDescriptors) {
      CFFileDescriptorInvalidate(fd);
      CFRelease(fd);
   }   
   
   CleanDescriptorArrays();
}

//______________________________________________________________________________
void MacOSXSystem::CleanDescriptorArrays()
{
   fCFFileDescriptors.clear();
}

}//Detail
}//MacOSX
}//ROOT

namespace Private = ROOT::MacOSX::Detail;

ClassImp(TMacOSXSystem)

//______________________________________________________________________________
TMacOSXSystem::TMacOSXSystem()
                  : fPimpl(new ROOT::MacOSX::Detail::MacOSXSystem)
{
   [NSApplication sharedApplication];
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
      //NSLog(@"got user event! fd is %d", int(event.data1));
      // nothing ready, so setup select call
      //*fReadready  = *fReadmask;
      //*fWriteready = *fWritemask;
      fReadready->Zero();
      fWriteready->Zero();
      fNfd = 1;
      
      auto fdIter = fPimpl->fFileDescriptors.find(event.data1);
      assert(fdIter != fPimpl->fFileDescriptors.end() && "WaitForAllEvents, file descriptor from NSEvent not found");
      if (fdIter->second == Private::MacOSXSystem::DescriptorType::read) {
         fReadready->Set(event.data1);
      } else {
         fWriteready->Set(event.data1);
      }
                     
      fPimpl->CleanDescriptorArrays();
   } else {
      [NSApp sendEvent : event];
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
   fPimpl->AddFileHandler(fh);
   TUnixSystem::AddFileHandler(fh);
}

//______________________________________________________________________________
TFileHandler *TMacOSXSystem::RemoveFileHandler(TFileHandler *fh)
{
   fPimpl->RemoveFileHandler(fh);
   return TUnixSystem::RemoveFileHandler(fh);
}
