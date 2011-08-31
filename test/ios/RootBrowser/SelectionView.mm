//
//  SelectionView.m
//  root_browser
//
//  Created by Timur Pocheptsov on 8/31/11.
//  Copyright 2011 __MyCompanyName__. All rights reserved.
//

#import <CoreGraphics/CGContext.h>

#import "SelectionView.h"
#import "PadView.h"

//C++ (ROOT) imports.
#import "IOSPad.h"

@implementation SelectionView

- (id)initWithFrame : (CGRect)frame withPad : (ROOT_iOS::Pad *) p
{
   self = [super initWithFrame:frame];

   if (self) {
      pad = p;
      self.opaque = NO;
   }

   return self;
}

- (void)drawRect : (CGRect)rect
{
   CGContextRef ctx = UIGraphicsGetCurrentContext();
   
   CGContextClearRect(ctx, rect);
   CGContextTranslateCTM(ctx, 0.f, rect.size.height);
   CGContextScaleCTM(ctx, 1.f, -1.f);
   
   pad->SetViewWH(rect.size.width, rect.size.height);
   pad->cd();
   pad->SetContext(ctx);

   CGContextTranslateCTM(ctx, 2.5f, 2.5f);
   pad->PaintShadowForSelected();
   CGContextTranslateCTM(ctx, -2.5f, -2.5f);
   pad->PaintSelected();
}


- (void)dealloc
{
    [super dealloc];
}

- (BOOL)pointInside:(CGPoint)point withEvent:(UIEvent *) event 
{
   //Thanks to gyim, 
   //http://stackoverflow.com/questions/1694529/allowing-interaction-with-a-uiview-under-another-uiview
   return NO;
}
 

@end
