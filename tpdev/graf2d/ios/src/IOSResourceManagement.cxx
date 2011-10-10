#include <stdexcept>

#include "IOSResourceManagement.h"

namespace ROOT {
namespace iOS {
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


}//namespace Util
}//namespace iOS
}//namespace ROOT
