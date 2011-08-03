//
//  Empty.h
//  Tutorials
//
//  Created by Timur Pocheptsov on 8/2/11.
//  Copyright 2011 __MyCompanyName__. All rights reserved.
//

#include <CoreGraphics/CGContext.h>

#ifdef __cplusplus
extern "C" {
#endif

CGContextRef GetImageContext(CGRect rect);

CGImageRef GetImageFromContext();
void FreeImage(CGImageRef im);

void EndImageContext();


#ifdef __cplusplus
}
#endif
