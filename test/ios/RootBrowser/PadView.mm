#import <stddef.h>
#import <string.h>
#import <stdlib.h>
#import <math.h>

#import <CoreGraphics/CoreGraphics.h>
#import <CoreGraphics/CGContext.h>

#import "ROOTObjectController.h"
#import "SelectionView.h"
#import "Constants.h"
#import "PadView.h"

#import "TAxis.h"

//C++ code (ROOT's ios module)
#import "IOSPad.h"

@implementation PadView

@synthesize selectionView;

//____________________________________________________________________________________________________
- (id) initWithFrame : (CGRect)frame controller : (ROOTObjectController *)c forPad : (ROOT_iOS::Pad*)pd
{
   self = [super initWithFrame : frame];

   if (self) {
      controller = c;
      pad = pd;
      
      frame.origin = CGPointZero;
      selectionView = [[SelectionView alloc] initWithFrame : frame withPad : pad];
      selectionView.autoresizingMask = UIViewAutoresizingFlexibleBottomMargin | UIViewAutoresizingFlexibleTopMargin | UIViewAutoresizingFlexibleLeftMargin | UIViewAutoresizingFlexibleRightMargin;
      selectionView.hidden = YES;
      [self addSubview : selectionView];
      [selectionView release];
   }

   return self;
}

//____________________________________________________________________________________________________
- (void) dealloc
{
   [super dealloc];
}

//____________________________________________________________________________________________________
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

//____________________________________________________________________________________________________
- (void) clearPad
{
   pad->Clear();
}

//____________________________________________________________________________________________________
- (void) addPanRecognizer
{
   panActive = YES;
}

//____________________________________________________________________________________________________
- (void) removePanRecognizer
{
   panActive = NO;
}

//____________________________________________________________________________________________________
- (void) handleDoubleTap
{
   //This is zoom/unzoom action.
   if (TAxis * axis = dynamic_cast<TAxis *>(pad->GetSelected())) {
   
      if (pad->ObjectInPoint(tapPt.x, tapPt.y) == axis) {
         axis->UnZoom();
         pad->InvalidateSelection();
         [self setNeedsDisplay];
         return;
      }
   }
   [controller handleDoubleTapOnPad];
}

#pragma mark - Picking related stuff here.

//____________________________________________________________________________________________________
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

//____________________________________________________________________________________________________
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

//____________________________________________________________________________________________________
- (BOOL) initPadPicking
{
   CGImageRef cgImage = [self initCGImageForPicking];
   if (!cgImage)
      return NO;

   const BOOL res = [self fillPickingBufferFromCGImage : cgImage];
   CGImageRelease(cgImage);
   
   return res;
}

//____________________________________________________________________________________________________
- (BOOL) pointOnSelectedObject : (CGPoint) pt
{
   //check if there is any object under pt.
   //this is just a test expression, let's say, there is an object selected in the corner.
//   return pt.x < 200 && pt.y < 200;

   const CGFloat scale = ROOT_IOSBrowser::padW / self.frame.size.width;
   const CGPoint newPt = CGPointMake(pt.x * scale, pt.y * scale);

   if (!pad->SelectionIsValid() && ![self initPadPicking])
      return NO;
      
   if (pad->GetSelected() == pad->ObjectInPoint(newPt.x, newPt.y))
      return YES;

   return NO;
}

//____________________________________________________________________________________________________
- (void) handleSingleTap
{
   //Make a selection, fill the editor.
   const CGFloat scale = ROOT_IOSBrowser::padW / self.frame.size.width;
   const CGPoint scaledTapPt = CGPointMake(tapPt.x * scale, tapPt.y * scale);

   if (!pad->SelectionIsValid() && ![self initPadPicking])
      return;
      
   pad->Pick(scaledTapPt.x, scaledTapPt.y);

   //Tell controller that selection has probably changed.
   [controller objectWasSelected : pad->GetSelected()];

   processSecondTap = NO;
}

//____________________________________________________________________________________________________
- (void) longPress
{
  // NSLog(@"long press");
   processFirstTap = NO;
   processSecondTap = NO;
   processLongPress = YES;

   //Select object here.
   UIScrollView *parent = (UIScrollView *)[self superview];
   parent.canCancelContentTouches = NO;
   parent.delaysContentTouches = NO;
}

//____________________________________________________________________________________________________
- (void) touchesBegan : (NSSet *)touches withEvent : (UIEvent *)event
{
   if (!padIsEditable)
      return;

   UITouch *touch = [touches anyObject];
   if (touch.tapCount == 1) {
      //Interaction has started.
      tapPt = [touch locationInView : self];
      //Gesture can be any of them:
      processFirstTap = YES;
      processSecondTap = YES;

      //Long press only after 2 seconds.
      processLongPress = NO;      
      [self performSelector:@selector(longPress) withObject:nil afterDelay : 1.f];
   } else if (touch.tapCount == 2) {
      [NSObject cancelPreviousPerformRequestsWithTarget : self];
   }
}

//____________________________________________________________________________________________________
- (void) touchesMoved : (NSSet *)touches withEvent : (UIEvent *)event
{
   if (!padIsEditable)
      return;

   if (panActive || processLongPress) {
      TObject *selected = pad->GetSelected();
      if (TAxis *axis = dynamic_cast<TAxis *>(selected)) {
         if (!selectionView.panActive) {
            selectionView.panActive = YES;
            if (!strcmp(axis->GetName(), "xaxis"))
               selectionView.verticalDirection = NO;
            else
               selectionView.verticalDirection = YES;
            selectionView.panStart = tapPt;
            
            pad->ExecuteEventAxis(kButton1Down, tapPt.x, tapPt.y, axis);
         } else {
            const CGPoint newPt = [[touches anyObject] locationInView : self];
            selectionView.currentPanPoint = newPt;
            pad->ExecuteEventAxis(kButton1Motion, newPt.x, newPt.y, axis);
            [selectionView setNeedsDisplay];
         }
      }
   }
}

//____________________________________________________________________________________________________
- (void) touchesEnded : (NSSet *)touches withEvent : (UIEvent *)event
{
   if (!padIsEditable)
      return;

   UITouch *touch = [touches anyObject];
   if (touch.tapCount == 1) {
      if (processFirstTap) {
         //Ok, longPress selector was not performed yet, let's cancell it.
         [NSObject cancelPreviousPerformRequestsWithTarget : self];
         //NSLog(@"cancel long press");
         //Still, we have to wait for a second tap.
         processSecondTap = YES;
         //tapPt = [touch locationInView : self];
         [self performSelector : @selector(handleSingleTap) withObject:nil afterDelay : 0.15];
      } else if (processLongPress) {
         //Finish the long press action.
         UIScrollView *parent = (UIScrollView *)[self superview];
         parent.canCancelContentTouches = YES;
         parent.delaysContentTouches = YES;
      }//else impossible.
   } else if (touch.tapCount == 2 && processSecondTap) {
     // [controller handleDoubleTapOnPad];
      [self handleDoubleTap];
   }
   
   if (selectionView.panActive) {
      selectionView.panActive = NO;
      const CGPoint pt = [touch locationInView : self];
      pad->ExecuteEventAxis(kButton1Up, pt.x, pt.y, (TAxis *)pad->GetSelected());
      pad->InvalidateSelection();
      [self setNeedsDisplay];
      [selectionView setNeedsDisplay];
   }
}

//____________________________________________________________________________________________________
- (BOOL) padIsEditable
{
   return padIsEditable;
}

//____________________________________________________________________________________________________
- (void) setPadIsEditable : (BOOL)ed
{
   padIsEditable = ed;
   if (!padIsEditable)
      selectionView.hidden = YES;
}

@end
