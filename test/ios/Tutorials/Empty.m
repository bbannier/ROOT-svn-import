//
//  Empty.m
//  Tutorials
//
//  Created by Timur Pocheptsov on 8/2/11.
//  Copyright 2011 __MyCompanyName__. All rights reserved.
//

#import <UIKit/UIKit.h>

#import "Empty.h"

//______________________________________________________________________________
CGContextRef GetImageContext(CGRect rect)
{
   UIGraphicsBeginImageContext(rect.size);
   // get the context for CoreGraphics
   return UIGraphicsGetCurrentContext();
}

UIImage * selectionImage;

//______________________________________________________________________________
CGImageRef GetImageFromContext()
{
	selectionImage = UIGraphicsGetImageFromCurrentImageContext();
   [selectionImage retain];
   CGImageRef imRef = selectionImage.CGImage;
   CGImageRetain(imRef);
   
   return imRef;
}

//______________________________________________________________________________
void FreeImage(CGImageRef im)
{
   CGImageRelease(im);
   [selectionImage release];
}

//______________________________________________________________________________
void EndImageContext()
{
   UIGraphicsEndImageContext();
}
