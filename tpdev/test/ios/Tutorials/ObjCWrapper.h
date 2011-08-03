
#ifdef __cplusplus

#include <CoreGraphics/CGContext.h>

extern "C" {

#else

#import <CoreGraphics/CGContext.h>

#endif

//
//Wrapper for Obj-C code to be used in Objective-C++.
//If I try to include UIKit into C++ source code, I have a lot of compilation errors,
//even if C++ is compiled as Objective-C++. So I have this temporary hack.
//

CGContextRef GetImageContext(CGRect rect);

CGImageRef GetImageFromContext();
void FreeImage(CGImageRef im);

void EndImageContext();


#ifdef __cplusplus
}
#endif
