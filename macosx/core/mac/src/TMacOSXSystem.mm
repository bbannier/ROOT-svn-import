//#define DEBUG_ROOT_COCOA

#include <stdexcept>
#include <vector>
#include <set>

#import <Cocoa/Cocoa.h>

#include "TMacOSXSystem.h"

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

      NSLog(@"Set file descriptor for %d", *fdIter);
      
      CFRunLoopAddSource(CFRunLoopGetMain(), runLoopSource.Get(), kCFRunLoopDefaultMode);

      fRunLoopSources.push_back(runLoopSource);      
      fCFFileDescriptors.push_back(fdref);
   }
   NSLog(@"SetFileDescriptors----------");
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
void TMacOSXSystem::DispatchOneEvent(Bool_t pendingOnly)
{
   //NSLog(@"DispatchOneEvent");
   //TUnixSystem::DispatchOneEvent(pendingOnly);
//
   try {
      fPimpl->SetFileDescriptors();
   } catch (const std::exception &e) {
#ifdef DEBUG_ROOT_COCOA
      NSLog(@"%s", e.what());
      throw;
#endif
      return;
   }
   
   NSEvent *event = [NSApp nextEventMatchingMask : NSAnyEventMask untilDate : [NSDate distantFuture] inMode : NSDefaultRunLoopMode dequeue : YES];
#ifdef DEBUG_ROOT_COCOA
   NSLog(@"Got event %@", event);
#endif
   fPimpl->CloseFileDescriptors();
   
   //Now call base class implementation, not to break tty :)
   TUnixSystem::DispatchOneEvent(pendingOnly);
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
