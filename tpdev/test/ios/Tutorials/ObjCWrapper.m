#import <UIKit/UIKit.h>

#import "ObjCWrapper.h"

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
