#import <cassert>

#import "QuartzPixmap.h"

@implementation QuartzPixmap {
@private
   unsigned fWidth;
   unsigned fHeight;
   
   CGContextRef fContext;
}

@synthesize fID;

//______________________________________________________________________________
- (id) initWithSize : (NSSize) pixmapSize
{
   if (self = [super init]) {
      fWidth = 0;
      fHeight = 0;
      
      if ([self resize : pixmapSize])
         return self;
   }

   //Yes, if context creation failed, the calling code should use
   //separate alloc/init statements to check this.
   return nil;
}

//______________________________________________________________________________
- (void) dealloc
{
   if (fContext)
      CGContextRelease(fContext);
   [super dealloc];
}

//______________________________________________________________________________
- (BOOL) resize : (NSSize) newSize
{
   assert(newSize.width > 0 && "Pixmap width must be positive");
   assert(newSize.height > 0 && "Pixmap height must be positive");

   CGColorSpaceRef colorSpace = CGColorSpaceCreateDeviceRGB();//[1]

   if (!colorSpace) {
      assert(colorSpace && "CGColorSpaceCreateDeviceRGB failed");
      return NO;
   }
   
   CGContextRef ctx = CGBitmapContextCreate(nullptr, newSize.width, newSize.height, 8, 0, colorSpace, kCGImageAlphaPremultipliedLast);//[2]

   if (!ctx) {
      CGColorSpaceRelease(colorSpace);//[1], ![2]
      assert(ctx && "CGBitmapContextCreate failed");
      return NO;
   }

   //
   //For debug only: fill bitmap with green color.
   //
   //CGContextSetRGBFillColor(ctx, 0.f, 1.f, 0.f, 1.f);
   //CGContextFillRect(ctx, CGRectMake(0.f, 0.f, newSize.width, newSize.height));

   
   if (fContext) {
      //New context was created OK, we can release now the old one.
      CGContextRelease(fContext);//[2]
   }

   //Size to be used later - to identify,
   //if we really have to resize.
   fWidth = newSize.width;
   fHeight = newSize.height;
   
   fContext = ctx;//[2]

   CGColorSpaceRelease(colorSpace);//[1]

   return YES;

}

//______________________________________________________________________________
- (BOOL) fIsPixmap
{
   return YES;
}

//______________________________________________________________________________
- (unsigned) fWidth
{
   assert(fContext && "fWidth, called for bad pixmap");
   return fWidth;
}

//______________________________________________________________________________
- (unsigned) fHeight
{
   assert(fContext != nullptr && "fHeight, called for bad pixmap");
   return fHeight;
}

//______________________________________________________________________________
- (NSSize) fSize
{
   NSSize size = {};
   size.width = fWidth;
   size.height = fHeight;
   return size;
}

//______________________________________________________________________________
- (CGContextRef) fContext
{
   assert(fContext != nullptr && "fContext, called for bad pixmap");   
   return fContext;
}

@end
