#include <iostream>

#import <Cocoa/Cocoa.h>

#include "TMacOSXSystem.h"

class TMacOSXSystemPrivate {
   //
};

ClassImp(TMacOSXSystem)

//______________________________________________________________________________
TMacOSXSystem::TMacOSXSystem()
                  : fPimpl(new TMacOSXSystemPrivate)
{
//   std::cout<<"IGOGO\n";
   NSLog(@"igogo");
}

//______________________________________________________________________________
TMacOSXSystem::~TMacOSXSystem()
{
}

//______________________________________________________________________________
void TMacOSXSystem::DispatchOneEvent(Bool_t pendingOnly)
{
   //
   TUnixSystem::DispatchOneEvent(pendingOnly);
}
