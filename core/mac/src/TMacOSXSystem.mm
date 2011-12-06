//#define DEBUG_ROOT_COCOA

#include <stdexcept>
#include <vector>
#include <set>

#import <Cocoa/Cocoa.h>

#include "TSeqCollection.h"
#include "TMacOSXSystem.h"
#include "TError.h"

//
//Staff, which I have to copy from TUnixSystem. Find a better way to organize code.
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

namespace {

//
//This RAII class here is only temporarily. It should go into some generic utilities/be replaced by standard library component later.
//

template<class RefType>
class CFGuard {
public:
   CFGuard();
   explicit CFGuard(RefType ref, bool initRetain);
   CFGuard(const CFGuard &rhs);
   
   //TODO: Check ravlue references also.
   CFGuard &operator = (const CFGuard &rhs);
   CFGuard &operator = (RefType ref);

   ~CFGuard();
   
   RefType Get()const
   {
      return fFdRef;
   }
   
private:
   RefType fFdRef;
};

//______________________________________________________________________________
template<class RefType>
CFGuard<RefType>::CFGuard()
                    : fFdRef(0)
{
}

//______________________________________________________________________________
template<class RefType>
CFGuard<RefType>::CFGuard(RefType ref, bool initRetain)
                     : fFdRef(ref)
{
   if (initRetain)
      CFRetain(ref);
}

//______________________________________________________________________________
template<class RefType>
CFGuard<RefType>::CFGuard(const CFGuard &rhs)
{
   fFdRef = rhs.fFdRef;
   CFRetain(rhs.fFdRef);
}

//______________________________________________________________________________
template<class RefType>
CFGuard<RefType> &CFGuard<RefType>::operator = (const CFGuard &rhs)
{
   if (this != &rhs) {
      CFRelease(fFdRef);
      fFdRef = rhs.fFdRef;
      CFRetain(fFdRef);
   }

   return *this;
}

//______________________________________________________________________________
template<class RefType>
CFGuard<RefType> &CFGuard<RefType>::operator = (RefType ref)
{
   if (fFdRef != ref) {
      CFRelease(fFdRef);
      fFdRef = ref;
      CFRetain(ref);
   }
   
   return *this;
}

//______________________________________________________________________________
template<class RefType>
CFGuard<RefType>::~CFGuard()
{
   CFRelease(fFdRef);
}

//
//This RAII class should also go to some separate module, or be here and included by TGCocoa/TGQuartz. Right now, it's simply 
//

}

//
//Hidden implementation details (in particular, hidden from CINT).
//

class TMacOSXSystemPrivate {
   friend class TMacOSXSystem;
   std::set<int> fFileDescriptors;
   
   typedef CFGuard<CFFileDescriptorRef> cffile_type;
   std::vector<cffile_type> fCFFileDescriptors;

   typedef CFGuard<CFRunLoopSourceRef> cfrl_type;
   std::vector<cfrl_type> fRunLoopSources;
   
   void SetFileDescriptors();
   void CloseFileDescriptors();
};

//______________________________________________________________________________
void TMacOSXSystemPrivate::SetFileDescriptors()
{
#ifdef DEBUG_ROOT_COCOA
   NSLog(@"---------SetFileDescriptors");
#endif

   for (auto fdIter = fFileDescriptors.begin(), end = fFileDescriptors.end(); fdIter != end; ++fdIter) {
      cffile_type fdref(CFFileDescriptorCreate(kCFAllocatorDefault, *fdIter, false, TMacOSXSystem_ReadCallback, 0), false);//Check how to use red/write callbacks or one callback later.
      if (!fdref.Get())
         throw std::runtime_error("TMacOSXSystemPrivate::SetFileDescriptors: CFFileDescriptorCreate failed");

      CFFileDescriptorEnableCallBacks(fdref.Get(), kCFFileDescriptorReadCallBack);
   
      cfrl_type runLoopSource(CFFileDescriptorCreateRunLoopSource(kCFAllocatorDefault, fdref.Get(), 0), false);
      if (!runLoopSource.Get())
         throw std::runtime_error("TMacOSXSystemPrivate::SetFileDescriptors: CFFileDescriptorCreateRunLoopSource failed");
#ifdef DEBUG_ROOT_COCOA
      NSLog(@"Set file descriptor for %d", *fdIter);
#endif
      CFRunLoopAddSource(CFRunLoopGetMain(), runLoopSource.Get(), kCFRunLoopDefaultMode);

      fRunLoopSources.push_back(runLoopSource);      
      fCFFileDescriptors.push_back(fdref);
   }

#ifdef DEBUG_ROOT_COCOA
   NSLog(@"SetFileDescriptors----------");
#endif
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
         NSLog(@"Got event %@", event);
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

   NSEvent *event = [NSApp nextEventMatchingMask : NSAnyEventMask untilDate : [NSDate distantFuture] inMode : NSDefaultRunLoopMode dequeue : YES];

#ifdef DEBUG_ROOT_COCOA
   NSLog(@"Got event %@", event);
#endif
}

//______________________________________________________________________________
void TMacOSXSystem::WaitForAllEvents(Long_t nextto)
{
   //Wait for one event, do not dequeue (will be done by the following non-blocking call).
#ifdef DEBUG_ROOT_COCOA
   NSLog(@"blocking call now - WaitForAllEvents");
#endif

   try {
      fPimpl->SetFileDescriptors();
      
      NSEvent *event = [NSApp nextEventMatchingMask : NSAnyEventMask untilDate : [NSDate distantFuture] inMode : NSDefaultRunLoopMode dequeue : YES];
      
      if (event.type == NSApplicationDefined) {
         //TODO: this check is only for test, do it right later (somehow identify an event in a CFFileDescriptor).
         //Remove from event queue.
#ifdef DEBUG_ROOT_COCOA
         NSLog(@"got app defined event, try to remove from the queue");
#endif
         event = [NSApp nextEventMatchingMask : NSAnyEventMask untilDate : nil inMode : NSDefaultRunLoopMode dequeue : YES];

#ifdef DEBUG_ROOT_COCOA
         NSLog(@"???");
#endif

         int mxfd = TMath::Max(fMaxrfd, fMaxwfd);
         mxfd++;
         // nothing ready, so setup select call
         *fReadready  = *fReadmask;
         *fWriteready = *fWritemask;
         
         fNfd = UnixSelect(mxfd, fReadready, fWriteready, nextto);
         if (fNfd < 0 && fNfd != -2) {
            TFdSet t;
            for (int fd = 0; fd < mxfd; ++fd) {
               t.Set(fd);
               if (fReadmask->IsSet(fd)) {
                  const int rc = UnixSelect(fd + 1, &t, 0, 0);
                  if (rc < 0 && rc != -2) {
                     SysError("DispatchOneEvent", "select: read error on %d\n", fd);
                     fReadmask->Clr(fd);
                  }
               }
               if (fWritemask->IsSet(fd)) {
                  const int rc = UnixSelect(fd + 1, 0, &t, 0);
                  if (rc < 0 && rc != -2) {
                     SysError("DispatchOneEvent", "select: write error on %d\n", fd);
                     fWritemask->Clr(fd);
                  }
               }
               t.Clr(fd);
            }
         }
      } else {
#ifdef DEBUG_ROOT_COCOA
         NSLog(@"got a GUI (?) event %@", event);
#endif
      }
   
      fPimpl->CloseFileDescriptors();
   } catch (const std::exception &e) {
      fPimpl->CloseFileDescriptors();
#ifdef DEBUG_ROOT_COCOA
      NSLog(@"%s", e.what());
      throw;
#endif
      return;//Should somehow stop everything, I think, this can be considered as fatal problem.
   }

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
#ifdef DEBUG_ROOT_COCOA
   NSLog(@"TMacOSXSystem::AddFileHandler, file descriptor is: %d", fh->GetFd());
#endif

   if (fPimpl->fFileDescriptors.find(fh->GetFd()) == fPimpl->fFileDescriptors.end())
      fPimpl->fFileDescriptors.insert(fh->GetFd());
   else {
#ifdef DEBUG_ROOT_COCOA
      NSLog(@"File descriptor %d added twice", fh->GetFd());
#endif
      return;
   }

   TUnixSystem::AddFileHandler(fh);
}

//______________________________________________________________________________
TFileHandler *TMacOSXSystem::RemoveFileHandler(TFileHandler *fh)
{
#ifdef DEBUG_ROOT_COCOA
   NSLog(@"TMacOSXSystem::RemoveFileHandler, file descriptor is %d", fh->GetFd());
#endif

   auto fdIter = fPimpl->fFileDescriptors.find(fh->GetFd());
   if (fdIter != fPimpl->fFileDescriptors.end()) {
      fPimpl->fFileDescriptors.erase(fdIter);
   } else {
#ifdef DEBUG_ROOT_COCOA
      NSLog(@"Attempt to remove unregistered file handler!");
#endif
      return fh;
   }

   return TUnixSystem::RemoveFileHandler(fh);
}
