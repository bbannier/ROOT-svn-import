//Author: Timur Pocheptsov 16/02/2012

#import <algorithm>

#import <cassert>
#import <cstddef>

#import "QuartzPixmap.h"

@implementation QuartzPixmap {
@private
   unsigned fWidth;
   unsigned fHeight;
   
   CGContextRef fContext;
}

@synthesize fID;

//______________________________________________________________________________
- (id) initWithW : (unsigned) width H : (unsigned) height
{
   if (self = [super init]) {
      fWidth = 0;
      fHeight = 0;
      
      if ([self resizeW : width H : height])
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
- (BOOL) resizeW : (unsigned) width H : (unsigned) height
{
   assert(width > 0 && "Pixmap width must be positive");
   assert(height > 0 && "Pixmap height must be positive");

   CGColorSpaceRef colorSpace = CGColorSpaceCreateDeviceRGB();//[1]

   if (!colorSpace) {
      assert(colorSpace && "CGColorSpaceCreateDeviceRGB failed");
      return NO;
   }
   
   CGContextRef ctx = CGBitmapContextCreate(nullptr, width, height, 8, 0, colorSpace, kCGImageAlphaPremultipliedLast);//[2]

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
   fWidth = width;
   fHeight = height;
   
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

//Call backs for data provider.
extern "C" {

const void* ROOT_QuartzImage_GetBytePointer(void *info)
{
   assert(info != nullptr && "ROOT_QuartzImage_GetBytePointer, info parameter is null");
   return info;
}

void ROOT_QuartzImage_ReleaseBytePointer(void *, const void *)
{
   //Do nothing.
}


std::size_t ROOT_QuartzImage_GetBytesAtPosition(void* info, void* buffer, off_t position, std::size_t count)
{
    std::copy((char *)info + position, (char *)info + position + count, (char*)buffer);
    return count;
}

}


@implementation QuartzImage {
   unsigned fWidth;
   unsigned fHeight;
   
   CGImageRef fImage;
}

@synthesize fID;

//______________________________________________________________________________
- (id) initWithW : (unsigned) width H : (unsigned) height data : (unsigned char *)data
{
   //Two step initialization. If the second step (initWithW:....) fails, user must call release 
   //(after he checked the result of init call).

   assert(width != 0 && "initWithW:H:data:, width parameter is 0");
   assert(height != 0 && "initWithW:H:data:, height parameter is 0");
   assert(data != nullptr && "initWithW:H:data:, data parameter is null");

   if (self = [super init]) {
      const CGDataProviderDirectCallbacks providerCallbacks = {0, ROOT_QuartzImage_GetBytePointer, 
                                                               ROOT_QuartzImage_ReleaseBytePointer, 
                                                               ROOT_QuartzImage_GetBytesAtPosition, 0};

      //This w * h * 4 is ONLY for TGCocoa::CreatePixmapFromData.
      //If needed something else, I'll make this code more generic.
      CGDataProviderRef provider = CGDataProviderCreateDirect(data, width * height * 4, &providerCallbacks);
      if (!provider) {
         NSLog(@"initWithW:H:data: CGDataProviderCreateDirect failed");
         return nil;
      }
      
      //RGB - this is only for TGCocoa::CreatePixmapFromData.
      CGColorSpaceRef colorSpace = CGColorSpaceCreateDeviceRGB();
      if (!colorSpace) {
         NSLog(@"initWithW:H:data: CGColorSpaceCreateDeviceRGB failed");
         CGDataProviderRelease(provider);
         return nil;
      }
      
      //8 bits per component, 32 bits per pixel, 4 bytes per pixel, kCGImageAlphaLast:
      //all values hardcoded for TGCocoa::CreatePixmapFromData.
      fImage = CGImageCreate(width, height, 8, 32, width * 4, colorSpace, kCGImageAlphaLast, provider, 0, false, kCGRenderingIntentDefault);
      CGColorSpaceRelease(colorSpace);
      CGDataProviderRelease(provider);
      
      if (!fImage) {
         NSLog(@"initWithW:H:data: CGImageCreate failed");
         return nil;
      }

      fWidth = width;
      fHeight = height;

      return self;
   }
   
   return nil;
}

//______________________________________________________________________________
- (void) dealloc
{
   if (fImage)
      CGImageRelease(fImage);
   
   [super dealloc];
}

//______________________________________________________________________________
- (BOOL) fIsPixmap
{
   return YES;//??
}

//______________________________________________________________________________
- (unsigned) fWidth
{
   return fWidth;
}

//______________________________________________________________________________
- (unsigned) fHeight
{
   return fHeight;
}

//______________________________________________________________________________
- (CGImageRef) fImage
{
   return fImage;
}

@end
