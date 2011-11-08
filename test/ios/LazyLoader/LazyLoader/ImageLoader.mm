#import "ObjectShortcut.h"
#import "ImageLoader.h"

#import "FileUtils.h"
#import "IOSPad.h"

@interface ImageLoader () {
   ROOT::iOS::Browser::FileContainer *fileContainer;
   NSMutableSet *views;
}

@end

@implementation ImageLoader

//____________________________________________________________________________________________________
- (void) loadThumbnailForView : (UIView *)view 
{
   using namespace ROOT::iOS;
   
   if (fileContainer->GetObject(view.tag)) {
     // NSLog(@"%d", view.tag);
      Pad *pad = fileContainer->GetPadAttached(view.tag);
      //generate thumbnail.
      
      const CGRect rect = CGRectMake(0.f, 0.f, [ObjectShortcut imageWidth], [ObjectShortcut imageHeight]);
      UIGraphicsBeginImageContext(rect.size);
      CGContextRef ctx = UIGraphicsGetCurrentContext();
      if (!ctx) {
         UIGraphicsEndImageContext();
         return;
      }
      
      //Now draw into this context.
      CGContextTranslateCTM(ctx, 0.f, rect.size.height);
      CGContextScaleCTM(ctx, 1.f, -1.f);
      
      //Fill bitmap with white first.
      CGContextSetRGBFillColor(ctx, 0.f, 0.f, 0.f, 1.f);
      CGContextFillRect(ctx, rect);
      //Set context and paint pad's contents
      //with special colors (color == object's identity)
      pad->cd();
      pad->SetViewWH(rect.size.width, rect.size.height);
      pad->SetContext(ctx);
      pad->PaintThumbnail();


     // ObjectShortcut *sh = (ObjectShortcut *)view;
     // sh.image = UIGraphicsGetImageFromCurrentImageContext();
      UIImage *im = UIGraphicsGetImageFromCurrentImageContext();
     // NSLog(@"created imaged for tag %d", view.tag);
   
      UIGraphicsEndImageContext();
   }
}

- (id) initWithContainer : (ROOT::iOS::Browser::FileContainer *)container andViews : (NSMutableSet *)vs
{
   if (self = [super init]) {
      fileContainer = container;
      views = vs;
   }
   
   return self;
}

- (void) main 
{
   NSLog(@"started");
   for (UIView * v in views) {
      [self loadThumbnailForView : v];
      if ([self isCancelled]) {
         NSLog(@"cancelled");
         return;
      }
   }
   NSLog(@"finished");
}

@end
