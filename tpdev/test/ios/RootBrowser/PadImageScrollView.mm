#import "PadImageScrollView.h"
#import "PadImageView.h"

//C++ (ROOT) imports.
#import "TObject.h"
#import "IOSPad.h"

static const CGFloat maxZoom = 2.f;
static const CGFloat minZoom = 1.f;

@implementation PadImageScrollView

@synthesize padImage;

//____________________________________________________________________________________________________
+ (CGRect) defaultImageFrame
{
   return CGRectMake(0.f, 0.f, 700.f, 700.f);
}

//____________________________________________________________________________________________________
- (CGPoint) adjustOriginForFrame : (CGRect)frame withSize : (CGSize) sz
{
   return CGPointMake(frame.size.width / 2 - sz.width / 2, frame.size.height / 2 - sz.height / 2);
}

//____________________________________________________________________________________________________
- (void) initPadImageView : (CGRect)frame
{
   CGRect padFrame = [PadImageScrollView defaultImageFrame];
   padFrame.origin = [self adjustOriginForFrame : frame withSize : padFrame.size];
   
   nestedView = [[PadImageView alloc] initWithFrame : padFrame];
   nestedView.autoresizingMask = UIViewAutoresizingFlexibleBottomMargin | UIViewAutoresizingFlexibleTopMargin | UIViewAutoresizingFlexibleLeftMargin | UIViewAutoresizingFlexibleRightMargin;
   
   [self addSubview : nestedView];
   [nestedView release];
}

//____________________________________________________________________________________________________
- (void) setContentSize : (CGSize) size contentOffset : (CGPoint)offset minScale : (CGFloat)min maxScale : (CGFloat)max scale : (CGFloat)scale
{
   self.maximumZoomScale = max;
   self.minimumZoomScale = min;
   self.zoomScale = scale;
   self.contentSize = size;
   self.contentOffset = offset;
}

//____________________________________________________________________________________________________
- (id) initWithFrame : (CGRect)frame andPad : (ROOT_iOS::Pad *)p
{
   if (self = [super initWithFrame : frame]) {
      pad = p;
      self.delegate = self; //hehehehe
      self.bouncesZoom = NO;
      self.backgroundColor = [UIColor clearColor];

      [self setContentSize : frame.size contentOffset : CGPointZero minScale : minZoom maxScale : maxZoom scale : 1];
   }
    
   return self;
}

//____________________________________________________________________________________________________
- (void) dealloc 
{
   [padImage release];
   [super dealloc];
}

#pragma mark - Child view's management
//____________________________________________________________________________________________________
- (void) clearScroll
{
   [self setContentSize : [PadImageScrollView defaultImageFrame].size contentOffset : CGPointZero minScale : minZoom maxScale : maxZoom scale : 1];
   [nestedView removeFromSuperview];
   nestedView = nil;
}

#pragma mark - Image/pad/geometry management.
//____________________________________________________________________________________________________
- (UIImage *) generatePadImage : (CGSize) imageSize
{
   UIGraphicsBeginImageContext(imageSize);
   CGContextRef ctx = UIGraphicsGetCurrentContext();

   if (!ctx) {
      NSLog(@"UIGraphicsGetCurrentContext failed");
      UIGraphicsEndImageContext();
      return nil;
   }

   //Now draw into this context.
   CGContextTranslateCTM(ctx, 0.f, imageSize.height);
   CGContextScaleCTM(ctx, 1.f, -1.f);
      
   const CGRect imageFrame = CGRectMake(0.f, 0.f, imageSize.width, imageSize.height);
   //Fill bitmap with white first.
   CGContextSetRGBFillColor(ctx, 1.f, 1.f, 1.f, 1.f);
   CGContextFillRect(ctx, imageFrame);
   //
   pad->cd();
   pad->Clear();
   pad->SetContext(ctx);
   pad->SetViewWH(imageSize.width, imageSize.height);
   object->Draw(drawOption);
   pad->Paint();

   //Ready.
   UIImage *image = UIGraphicsGetImageFromCurrentImageContext();//autoreleased UIImage.
   [image retain];
   UIGraphicsEndImageContext();
       
   return image;
}

//____________________________________________________________________________________________________
- (void) setObject : (TObject *)obj drawOption : (const char *)opt
{
   object = obj;
   drawOption = opt;

   //Generate new image.
   [padImage release];
   padImage = [self generatePadImage : [PadImageScrollView defaultImageFrame].size];

   if (nestedView && nestedView.zoomed) {
      [self clearScroll];
      [self initPadImageView : self.frame];
   } else if (!nestedView) {
      [self initPadImageView : self.frame];
   }

   nestedView.padImage = padImage;
   [nestedView setNeedsDisplay];
}

//____________________________________________________________________________________________________
- (void) setObject : (TObject *)obj drawOption : (const char *) opt andImage : (UIImage *)image
{
   object = obj;
   drawOption = opt;
   
   [padImage release];
   padImage = [image retain];
   
   if (nestedView && nestedView.zoomed) {
      [self clearScroll];
      [self initPadImageView : self.frame];
      
   } else if (!nestedView) {
      [self initPadImageView : self.frame];
   }
   
   nestedView.padImage = padImage;
   [nestedView setNeedsDisplay];
}

//____________________________________________________________________________________________________
- (void) resetToFrame : (CGRect) newFrame
{
   self.frame = newFrame; 
   [self setContentSize : newFrame.size contentOffset : CGPointZero minScale : minZoom maxScale : maxZoom scale : 1];
   
   if (nestedView.zoomed) {
      [self clearScroll];
      [self initPadImageView : newFrame];
      nestedView.padImage = padImage;
      [nestedView setNeedsDisplay];
   } else {
      CGRect padFrame = [PadImageScrollView defaultImageFrame];
      padFrame.origin = [self adjustOriginForFrame : newFrame withSize : padFrame.size];
   }
}

//
//_________________________________________________________________
- (CGRect)centeredFrameForScrollView:(UIScrollView *)scroll andUIView:(UIView *)rView 
{
   CGSize boundsSize = scroll.bounds.size;
   CGRect frameToCenter = rView.frame;
   // center horizontally
   if (frameToCenter.size.width < boundsSize.width) {
      frameToCenter.origin.x = (boundsSize.width - frameToCenter.size.width) / 2;
   }
   else {
      frameToCenter.origin.x = 0;
   }
   // center vertically
   if (frameToCenter.size.height < boundsSize.height) {
      frameToCenter.origin.y = (boundsSize.height - frameToCenter.size.height) / 2;
   }
   else {
      frameToCenter.origin.y = 0;
   }
   
   return frameToCenter;
}

//____________________________________________________________________________________________________
- (void)scrollViewDidZoom:(UIScrollView *)scroll
{
   nestedView.frame = [self centeredFrameForScrollView : scroll andUIView : nestedView];
}

//____________________________________________________________________________________________________
- (void)scrollViewDidEndZooming : (UIScrollView *)scroll withView : (UIView *)view atScale : (float)scale
{
   const CGPoint offset = [scroll contentOffset];
   const CGRect newFrame = nestedView.frame;
  
   [scroll setZoomScale : 1.f];
   
   const unsigned base = [PadImageScrollView defaultImageFrame].size.width;

   scroll.minimumZoomScale = base / newFrame.size.width;
   scroll.maximumZoomScale = maxZoom * base / newFrame.size.width;

   [nestedView removeFromSuperview];

   nestedView = [[PadImageView alloc] initWithFrame : newFrame];
   
   UIImage *image = [self generatePadImage : newFrame.size];
   nestedView.padImage = image;
   [image release];
   [scroll addSubview : nestedView];

   scroll.contentSize = newFrame.size;
   scroll.contentOffset = offset;

   nestedView.zoomed = YES;

   [nestedView release];
}

//____________________________________________________________________________________________________
- (UIView *) viewForZoomingInScrollView:(UIScrollView *)scrollView
{
   return nestedView;
}

@end
