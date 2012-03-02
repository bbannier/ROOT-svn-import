//Author: Timur Pocheptsov 16/02/2012

#import <algorithm>

#import <cstdlib>
#import <cassert>
#import <cstddef>

#import "QuartzPixmap.h"

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

@implementation QuartzPixmap {
@private
   unsigned       fWidth;
   unsigned       fHeight;
   unsigned char *fData;
   CGContextRef   fContext;
}

@synthesize fID;

//______________________________________________________________________________
- (id) initWithW : (unsigned) width H : (unsigned) height
{
   if (self = [super init]) {
      fWidth = 0;
      fHeight = 0;
      fData = 0;
      
      if ([self resizeW : width H : height])
         return self;
   }

   //Two step initialization:
   //1. p = [QuartzPixmap alloc];
   //2. p1 = [p initWithW : w H : h];
   // if (!p1) [p release];
   return nil;
}

//______________________________________________________________________________
- (void) dealloc
{
   if (fContext)
      CGContextRelease(fContext);
   if (fData)
      std::free(fData);

   [super dealloc];
}

//______________________________________________________________________________
- (BOOL) resizeW : (unsigned) width H : (unsigned) height
{
   assert(width > 0 && "Pixmap width must be positive");
   assert(height > 0 && "Pixmap height must be positive");

   unsigned char *memory = (unsigned char *)malloc(width * height * 4);//[0]
   if (!memory) {
      assert(0 && "resizeW:H:, malloc failed");
      return NO;
   }

   CGColorSpaceRef colorSpace = CGColorSpaceCreateDeviceRGB();//[1]
   if (!colorSpace) {
      assert(0 && "resizeW:H:, CGColorSpaceCreateDeviceRGB failed");
      std::free(memory);
      return NO;
   }

   //
   CGContextRef ctx = CGBitmapContextCreateWithData(memory, width, height, 8, width * 4, colorSpace, kCGImageAlphaPremultipliedLast, NULL, 0);
   if (!ctx) {
      assert(0 && "resizeW:H:, CGBitmapContextCreateWidthData failed");
      CGColorSpaceRelease(colorSpace);
      std::free(memory);
      return NO;
   }

   if (fContext) {
      //New context was created OK, we can release now the old one.
      CGContextRelease(fContext);//[2]
   }
   
   if (fData) {
      //Release old memory.
      std::free(fData);
   }

   //Size to be used later - to identify,
   //if we really have to resize.
   fWidth = width;
   fHeight = height;
   fData = memory;

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
   assert(fContext != nullptr && "fWidth, called for bad pixmap");

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

//______________________________________________________________________________
- (unsigned char *) fData
{
   return fData;
}

//______________________________________________________________________________
- (CGImageRef) createImageFromPixmap
{
   //
   const CGDataProviderDirectCallbacks providerCallbacks = {0, ROOT_QuartzImage_GetBytePointer, 
                                                            ROOT_QuartzImage_ReleaseBytePointer, 
                                                            ROOT_QuartzImage_GetBytesAtPosition, 0};

   
   CGDataProviderRef provider = CGDataProviderCreateDirect(fData, fWidth * fHeight * 4, &providerCallbacks);
   if (!provider) {
      NSLog(@"pixmapToImage, CGDataProviderCreateDirect failed");
      return nullptr;
   }

   //RGB - this is only for TGCocoa::CreatePixmapFromData.
   CGColorSpaceRef colorSpace = CGColorSpaceCreateDeviceRGB();
   if (!colorSpace) {
      NSLog(@"pixmapToImage, CGColorSpaceCreateDeviceRGB failed");
      CGDataProviderRelease(provider);
      return nullptr;
   }
      
   //8 bits per component, 32 bits per pixel, 4 bytes per pixel, kCGImageAlphaLast:
   //all values hardcoded for TGCocoa.
   CGImageRef image = CGImageCreate(fWidth, fHeight, 8, 32, fWidth * 4, colorSpace, kCGImageAlphaPremultipliedLast, provider, 0, false, kCGRenderingIntentDefault);
   CGColorSpaceRelease(colorSpace);
   CGDataProviderRelease(provider);
   
   return image;
}

@end

@implementation QuartzImage {
   unsigned fWidth;
   unsigned fHeight;
   
   CGImageRef fImage;
   unsigned char *fImageData;
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

      fImageData = data;

      return self;
   }
   
   return nil;
}

//______________________________________________________________________________
- (void) dealloc
{
   if (fImage) {
      CGImageRelease(fImage);
      delete [] fImageData;
   }
   
   [super dealloc];
}

//______________________________________________________________________________
- (BOOL) isRectInside : (Rectangle_t) area
{
   if (area.fX < 0 || (unsigned)area.fX >= fWidth)
      return NO;
   if (area.fY < 0 || (unsigned)area.fY >= fHeight)
      return NO;
   if (area.fWidth > fWidth || !area.fWidth)
      return NO;
   if (area.fHeight > fHeight || !area.fHeight)
      return NO;
   
   return YES;
}

//______________________________________________________________________________
- (void) readColorBits : (Rectangle_t) area intoBuffer : (unsigned char *) buffer
{
   assert([self isRectInside : area] == YES && "readColorBits:intoBuffer: bad area parameter");

   const unsigned char * line = fImageData + area.fY * fWidth * 4;
   const unsigned char *pixel = line + area.fX * 4;
   
   for (UShort_t i = 0; i < area.fHeight; ++i) {
      for (UShort_t j = 0; j < area.fWidth; ++j, pixel += 4) {
         buffer[0] = pixel[2];
         buffer[1] = pixel[1];
         buffer[2] = pixel[0];
         buffer[3] = pixel[3];
      }

      line += fWidth * 4;
      pixel = line + area.fX * 4;
   }
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
