#include <math.h>

#import <CoreGraphics/CoreGraphics.h>

#import "SelectionView.h"
#import "CppWrapper.h"
#import "PadView.h"

@implementation PadView

//_________________________________________________________________
- (id) initWithFrame:(CGRect)frame forPad:(PadWrapper*)padWrap withFontManager:(FontManagerWrapper*)fm andPainter:(PainterWrapper*)painterWrap;
{
   self = [super initWithFrame : frame];

   if (self) {
      //Initialize C++ objects here.
      pad = padWrap;
      fontManager = fm;
      painter = painterWrap;

      scaleFactor = frame.size.width / 640.f;
   }

   return self;
}

//_________________________________________________________________
- (void) dealloc
{
   [super dealloc];
}

//_________________________________________________________________
- (void)drawRect:(CGRect)rect
{
   // Drawing code
   CGContextRef ctx = UIGraphicsGetCurrentContext();

   CGContextTranslateCTM(ctx, 0.f, rect.size.height);
   CGContextScaleCTM(ctx, 1.f, -1.f);

   CGContextScaleCTM(ctx, scaleFactor, scaleFactor);

   pad->cd();
   painter->SetContext(ctx);
   pad->Paint();
}

//_________________________________________________________________
- (void) clearPad
{
   pad->Clear();
}

//_________________________________________________________________
- (void) handlePanGesture : (UIPanGestureRecognizer *)panGesture
{
   if (!processPan)
      return;

   const CGPoint p = [panGesture locationInView:self];
   [sv setPad:pad andPainter:painter];
   [sv setShowRotation:YES];
   
   if (panGesture.state == UIGestureRecognizerStateBegan) {
      sv.hidden = NO;
      [sv setEvent:PadWrapper::pwButton1Down atX:p.x andY:p.y];
      [sv setNeedsDisplay];
   } else if (panGesture.state == UIGestureRecognizerStateChanged) {
      [sv setEvent:PadWrapper::pwButton1Motion atX:p.x andY:p.y];
      [sv setNeedsDisplay];
   } else if (panGesture.state == UIGestureRecognizerStateEnded) {
      [sv setEvent:PadWrapper::pwButton1Up atX:p.x andY:p.y];
      [sv setNeedsDisplay];
      sv.hidden = YES;
      [self setNeedsDisplay];
   }
}

//_________________________________________________________________
- (void) handleTapGesture : (UITapGestureRecognizer *) tapGesture
{
   if (processTap) {
      const CGPoint tapPt = [tapGesture locationInView : self];
      pad->Pick(tapPt.x, tapPt.y);
      if (pad->Selected()) {
         [sv setShowRotation : NO];
         [sv setPad : pad andPainter : painter];
         [sv setNeedsDisplay];
         sv.hidden = NO;
      } else {
         sv.hidden = YES;
      }
   }
}

//_________________________________________________________________
- (void) setSelectionView:(SelectionView *)selView
{
   sv = selView;
}

//_________________________________________________________________
- (void) setProcessPan : (BOOL) p
{
   processPan = p;
}

//_________________________________________________________________
- (void) setProcessTap : (BOOL) t
{
   processTap = t;
}

@end
