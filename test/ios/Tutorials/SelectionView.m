//
//  SelectionView.m
//  Tutorials
//
//  Created by Timur Pocheptsov on 7/22/11.
//  Copyright 2011 __MyCompanyName__. All rights reserved.
//

#import "SelectionView.h"
#import "CppWrapper.h"


@implementation SelectionView

- (id)initWithFrame:(CGRect)frame
{
    self = [super initWithFrame:frame];
    if (self) {
        // Initialization code
        self.opaque = NO;
    }
    return self;
}

- (void)dealloc
{
    [super dealloc];
}

- (void) setPad : (PadWrapper *)newPad andPainter : (PainterWrapper *) newPainter
{
   pad = newPad;
   painter = newPainter;
}

- (void) setEvent : (int) e atX : (int) x andY : (int) y
{
   ev = e;
   px = x;
   py = y;
}

- (void) drawRect:(CGRect)rect
{
   if (!pad || !painter)
      return;

   CGContextRef ctx = UIGraphicsGetCurrentContext();
   CGContextClearRect(ctx, rect);

//   if (!showRotation)
//      CGContextTranslateCTM(ctx, -3.f, -3.f);

   CGContextTranslateCTM(ctx, 0.f, rect.size.height);
   CGContextScaleCTM(ctx, 1.f, -1.f);
   
   pad->cd();
   painter->SetContext(ctx);
   if (showRotation)
      pad->ExRot(ev, px, py);
   else
      pad->PaintSelected();
}

- (void) setShowRotation : (BOOL) show
{
   showRotation = show;
}

- (BOOL)pointInside:(CGPoint)point withEvent:(UIEvent *) event 
{
   //Thanks to gyim, 
   //http://stackoverflow.com/questions/1694529/allowing-interaction-with-a-uiview-under-another-uiview
   return NO;
}

@end
