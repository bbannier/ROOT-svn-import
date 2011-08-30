#import <stddef.h>
#import <stdlib.h>
#import <math.h>

#import <CoreGraphics/CoreGraphics.h>
#import <CoreGraphics/CGContext.h>

//#import "SelectionView.h"
#import "ROOTObjectController.h"
#import "PadView.h"

//C++ code (ROOT's ios module)
#import "IOSPad.h"

@implementation PadView

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
/*
//_________________________________________________________________
- (void) turnOnEditMode
{
   [self addGestureRecognizer : singleTap];
   [self addGestureRecognizer : doubleTap];
   [self addGestureRecognizer : longPress];
}

//_________________________________________________________________
- (void) turnOffEditoMode
{
   [self removeGestureRecognizer : singleTap];
   [self removeGestureRecognizer:doubleTap];
   [self removeGestureRecognizer:longPress];
   
   if (panActive)
      [self removeGestureRecognizer : pan];
}*/

//_________________________________________________________________
- (void) handleSingleTap : (UITapGestureRecognizer*)tap
{
   //Make a selection, fill the editor.
   NSLog(@"single tap");
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

@end
