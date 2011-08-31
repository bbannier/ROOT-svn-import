#import <stddef.h>
#import <stdlib.h>
#import <math.h>

#import <CoreGraphics/CoreGraphics.h>
#import <CoreGraphics/CGContext.h>

#import "ROOTObjectController.h"
#import "SelectionView.h"
#import "Constants.h"
#import "PadView.h"

//C++ code (ROOT's ios module)
#import "IOSPad.h"
//FOR TEST PURPOSE ONLY:
#import "TObject.h"
#import "TClass.h"

@implementation PadView

@synthesize selectionView;

//_________________________________________________________________
- (id) initWithFrame : (CGRect)frame controller : (ROOTObjectController *)c forPad : (ROOT_iOS::Pad*)pd
{
   self = [super initWithFrame : frame];

   if (self) {
      controller = c;
      pad = pd;
      
      //Create, but do not add.
      pan = [[UIPanGestureRecognizer alloc] initWithTarget : self action : @selector(handlePan:)];
      
      singleTap = [[UITapGestureRecognizer alloc] initWithTarget : self action : @selector(handleSingleTap:)];
      [singleTap setNumberOfTapsRequired : 1];
      doubleTap = [[UITapGestureRecognizer alloc] initWithTarget : self action : @selector(handleDoubleTap:)];
      [doubleTap setNumberOfTapsRequired : 2];
      [singleTap requireGestureRecognizerToFail : doubleTap];
      
      longPress = [[UILongPressGestureRecognizer alloc] initWithTarget:self action:@selector(handleLongPressGesture:)];
      
      [self addGestureRecognizer : singleTap];
      [self addGestureRecognizer : doubleTap];
      [self addGestureRecognizer : longPress];
      //
      frame.origin = CGPointZero;
      selectionView = [[SelectionView alloc] initWithFrame : frame withPad : pad];
      selectionView.autoresizingMask = UIViewAutoresizingFlexibleBottomMargin | UIViewAutoresizingFlexibleTopMargin | UIViewAutoresizingFlexibleLeftMargin | UIViewAutoresizingFlexibleRightMargin;
      selectionView.hidden = YES;
      [self addSubview : selectionView];
      [selectionView release];
   }

   return self;
}

//_________________________________________________________________
- (void) dealloc
{
   [pan release];
   [singleTap release];
   [doubleTap release];
   [longPress release];
   [super dealloc];
}

//_________________________________________________________________
- (void)drawRect : (CGRect)rect
{
   // Drawing code
   CGContextRef ctx = UIGraphicsGetCurrentContext();

   CGContextClearRect(ctx, rect);
   
   pad->SetViewWH(rect.size.width, rect.size.height);

   CGContextTranslateCTM(ctx, 0.f, rect.size.height);
   CGContextScaleCTM(ctx, 1.f, -1.f);

   pad->cd();
   pad->SetContext(ctx);
   pad->Paint();
   
   if (!selectionView.hidden)
      [selectionView setNeedsDisplay];
}

//_________________________________________________________________
- (void) clearPad
{
   pad->Clear();
}

//_________________________________________________________________
- (BOOL) pointOnSelectedObject : (CGPoint) pt
{
   //check if there is any object under pt.
   //this is just a test expression, let's say, there is an object selected in the corner.
   return pt.x < 200 && pt.y < 200;
}

//_________________________________________________________________
- (void) addPanRecognizer
{
   if (!panActive) {
      panActive = YES;
      [self addGestureRecognizer : pan];
   }
}

//_________________________________________________________________
- (void) removePanRecognizer
{
   if (panActive) {
      panActive = NO;
      [self removeGestureRecognizer : pan];
   }
}

//_________________________________________________________________
- (void) handleDoubleTap : (UITapGestureRecognizer*)tap
{
   //This is zoom/unzoom action.
   //NSLog(@"double tap");
   [controller handleDoubleTapOnPad];
}

//_________________________________________________________________
- (void) handlePan : (UIPanGestureRecognizer*)panGesture
{
   //const CGPoint panPoint = [panGesture locationInView : self];
   //Move (if can) the selected object.
   NSLog(@"pan in pad view works");
}

//_________________________________________________________________
- (void) handleLongPressGesture : (UILongPressGestureRecognizer *)press
{
   //const CGPoint pressPoint = [longPress locationInView : self];
   //Select the object under press.
   if (press.state == UIGestureRecognizerStateBegan) {
      NSLog(@"try to select");
      //Do selection.
   } else if (press.state == UIGestureRecognizerStateChanged) {
      NSLog(@"move the selected object");
      //Move the selected object, if possible.
   } else if (press.state == UIGestureRecognizerStateEnded) {
      //
      NSLog(@"stop the motion");
   }
   //Move it, if possible.
}

#pragma mark - Picking related stuff here.

//_________________________________________________________________
- (CGImageRef) initCGImageForPicking
{
   using namespace ROOT_IOSBrowser;
   const CGRect rect = CGRectMake(0.f, 0.f, padW, padH);
   //Create bitmap context.
   UIGraphicsBeginImageContext(rect.size);
   CGContextRef ctx = UIGraphicsGetCurrentContext();

   //Now draw into this context.
   CGContextTranslateCTM(ctx, 0.f, rect.size.height);
   CGContextScaleCTM(ctx, 1.f, -1.f);
      
   //Disable anti-aliasing, to avoid "non-clear" colors.
   CGContextSetAllowsAntialiasing(ctx, 0);
   //Fill bitmap with black (nothing under cursor).
   CGContextSetRGBFillColor(ctx, 0.f, 0.f, 0.f, 1.f);
   CGContextFillRect(ctx, rect);
   //Set context and paint pad's contents
   //with special colors (color == object's identity)
   pad->SetViewWH(rect.size.width, rect.size.height);

   pad->cd();

   pad->SetContext(ctx);
   pad->PaintForSelection();
   
   UIImage *uiImageForPicking = UIGraphicsGetImageFromCurrentImageContext();//autoreleased UIImage.
   CGImageRef cgImageForPicking = uiImageForPicking.CGImage;
   CGImageRetain(cgImageForPicking);//It must live as long, as I need :)
   
   UIGraphicsEndImageContext();
   
   return cgImageForPicking;

} 

//_________________________________________________________________
- (BOOL) fillPickingBufferFromCGImage : (CGImageRef) cgImage
{
	const size_t pixelsW = CGImageGetWidth(cgImage);
	const size_t pixelsH = CGImageGetHeight(cgImage);
	//Declare the number of bytes per row. Each pixel in the bitmap
	//is represented by 4 bytes; 8 bits each of red, green, blue, and
	//alpha.
	const int bitmapBytesPerRow = pixelsW * 4;
	const int bitmapByteCount = bitmapBytesPerRow * pixelsH;
	
	//Use the generic RGB color space.
	CGColorSpaceRef colorSpace = CGColorSpaceCreateDeviceRGB();
	if (!colorSpace) {
      //Log error: color space allocation failed.
      return NO;
   }
	
   unsigned char *buffer = (unsigned char*)malloc(bitmapByteCount);
   if (!buffer) {
      //Log error: memory allocation failed.
      CGColorSpaceRelease(colorSpace);
      return NO;
   }

	// Create the bitmap context. We want pre-multiplied ARGB, 8-bits 
	// per component. Regardless of what the source image format is 
	// (CMYK, Grayscale, and so on) it will be converted over to the format
	// specified here by CGBitmapContextCreate.
   CGContextRef ctx = CGBitmapContextCreate(buffer, pixelsW, pixelsH, 8, bitmapBytesPerRow, colorSpace, kCGImageAlphaPremultipliedFirst);

   CGColorSpaceRelease(colorSpace);

	if (!ctx) {
      //Log error: bitmap context creation failed.
      free(buffer);
      return NO;
   }
	
	const CGRect rect = CGRectMake(0.f, 0.f, pixelsW, pixelsH); 
	//Draw the image to the bitmap context. Once we draw, the memory 
	//allocated for the context for rendering will then contain the 
	//raw image data in the specified color space.
   
   CGContextSetAllowsAntialiasing(ctx, 0);//Check, if I need this for a bitmap.
	CGContextDrawImage(ctx, rect, cgImage);

   pad->SetSelectionBuffer(pixelsW, pixelsH, buffer);
	// When finished, release the context
	CGContextRelease(ctx); 
   free(buffer);

   return YES;
}

//_________________________________________________________________
- (BOOL) initPadPicking
{
   CGImageRef cgImage = [self initCGImageForPicking];
   if (!cgImage)
      return NO;

   const BOOL res = [self fillPickingBufferFromCGImage : cgImage];
   CGImageRelease(cgImage);
   
   return res;
}

//_________________________________________________________________
- (void) handleSingleTap : (UITapGestureRecognizer*)tap
{
   //Make a selection, fill the editor.
   CGPoint tapPt = [tap locationInView : self];
   
   //Scale point to picking buffer sizes.
   const CGFloat scale = ROOT_IOSBrowser::padW / self.frame.size.width;
   tapPt.x *= scale;
   tapPt.y *= scale;

   if (!pad->SelectionIsValid() && ![self initPadPicking])
      return;
      
   pad->Pick(tapPt.x, tapPt.y);

   TObject * obj = pad->GetSelected();
      
   if (obj) {
      //show the selected object in a selection view.
      [selectionView setNeedsDisplay];
      selectionView.hidden = NO;
   } else {
      selectionView.hidden = YES;
   }
   
   //Tell controller that selection has probably changed.
}

@end
