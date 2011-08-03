//
//  ResourceManagement.cpp
//  test_static_test
//
//  Created by Timur Pocheptsov on 6/30/11.
//  Copyright 2011 __MyCompanyName__. All rights reserved.
//

#include <stdexcept>


#include "ResourceManagement.h"

namespace ROOT_iOS {
namespace Util {

//_________________________________________________________________
CFStringGuard::CFStringGuard(const char *text)
                  : fCFString(0)
{
   fCFString = CFStringCreateWithCString(kCFAllocatorDefault, text, kCFStringEncodingMacRoman);
   if (!fCFString)
      std::runtime_error("CFStringGuard: create failed");
}

//_________________________________________________________________
CFStringGuard::~CFStringGuard()
{
   CFRelease(fCFString);
}

//_________________________________________________________________
CFStringRef CFStringGuard::Get()const
{
   return fCFString;
}

//_________________________________________________________________
CGStateGuard::CGStateGuard(CGContextRef ctx)
               : fCtx(ctx)
{
   CGContextSaveGState(ctx);
}

//_________________________________________________________________
CGStateGuard::~CGStateGuard()
{
   CGContextRestoreGState(fCtx);
}


}
}