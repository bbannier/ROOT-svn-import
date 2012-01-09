#import <fstream>

#import <assert.h>

#import "CocoaPixmap.h"

@implementation CocoaPixmap {
   CGImageRef fImage;
}

@synthesize fCurrentContext;
@synthesize fWidth;
@synthesize fHeight;
@synthesize fWinID;

//______________________________________________________________________________
- (id) initWithSize : (NSSize) pixmapSize
{
   if (self = [super init]) {
      if (![self resizePixmap : pixmapSize]) {
         //WHAT?
         return nil;//Caller must remember to check the result of init, and clean up the result of alloc.
      }
   }
   
   return self;
}

//______________________________________________________________________________
- (void) dealloc
{
   if (fImage)
      CGImageRelease(fImage);
   if (fCurrentContext)
      CGContextRelease(fCurrentContext);
}

//______________________________________________________________________________
- (BOOL) resizePixmap : (NSSize) newSize
{
   NSLog(@"size: %g %g", newSize.width, newSize.height);
   
   assert(newSize.width > 0 && "Pixmap width must be positive");
   assert(newSize.height > 0 && "Pixmap height must be positive");
   
   CGColorSpaceRef colorSpace = CGColorSpaceCreateDeviceRGB();

   //Better management required.
   if (!colorSpace) {
      assert(colorSpace && "CGColorSpaceCreateDeviceRGB failed");
      return FALSE;
   }
   
   CGContextRef ctx = CGBitmapContextCreate(NULL, newSize.width, newSize.height, 8, 0, colorSpace, kCGImageAlphaPremultipliedLast);

   //Better management required.
   if (!ctx) {
      CGColorSpaceRelease(colorSpace);
      assert(ctx && "CGBitmapContextCreate failed");
      return FALSE;
   }
   
      static std::ofstream dump("out.txt");

   if (fCurrentContext) {
      dump<<"Releasing "<<fCurrentContext<<std::endl;
      CGContextRelease(fCurrentContext);
   }



   //Size to be used later - to identify,
   //if we really have to resize.
   fWidth = newSize.width;
   fHeight = newSize.height;
   
   fCurrentContext = ctx;


   dump<<"ResizePixmap, ctx "<<fCurrentContext<<std::endl;

   CGColorSpaceRelease(colorSpace);

   return YES;
}

//______________________________________________________________________________
-(CGImageRef) fImage 
{
   //For now, just create image when called.
   //Later, re-use image somehow.

   assert(fCurrentContext != 0 && "No CGContextRef to create image");

//   CGContextSetRGBFillColor(fCurrentContext, 1.f, 0.3f, 0.3f, 1.f);
//   CGContextFillRect(fCurrentContext, CGRectMake(0.f, 0.f, fWidth, fHeight));

   //1. Delete old image.
   if (fImage)
      CGImageRelease(fImage);
   //2. Create new image.
   fImage = CGBitmapContextCreateImage(fCurrentContext);

   return fImage;
}

#pragma mark - RootGUIElement protocol.

//______________________________________________________________________________
- (void) addChildView : (RootQuartzView *)child
{
   (void)child;
}

//______________________________________________________________________________
- (void) setParentView : (id<RootGUIElement>)parent
{
   (void)parent;
}

//______________________________________________________________________________
- (id<RootGUIElement>) parentView
{
   return nil;
}

//______________________________________________________________________________
- (NSView *) contentView
{
   return nil;
}

//______________________________________________________________________________
- (BOOL) fIsPixmap
{
   return YES;
}

@end
