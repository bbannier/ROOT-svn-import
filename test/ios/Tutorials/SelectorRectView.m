//
//  SelectorRectView.m
//  Tutorials
//
//  Created by Timur Pocheptsov on 8/9/11.
//  Copyright 2011 __MyCompanyName__. All rights reserved.
//

#import <QuartzCore/QuartzCore.h>

#import "SelectorRectView.h"


@implementation SelectorRectView

//______________________________________________________________________________
- (id)initWithFrame:(CGRect)frame
{
   self = [super initWithFrame:frame];

   if (self) {
      // Initialization code
   }

   return self;
}

//______________________________________________________________________________
- (void)drawRect:(CGRect)rect
{
   // Drawing code
   //Draw a rect, filled with pattern.
   CGContextRef ctx = UIGraphicsGetCurrentContext();

   //Draw white background.
   CGContextSetRGBFillColor(ctx, 1.f, 1.f, 1.f, 1.f);
   CGContextSetRGBStrokeColor(ctx, 0.f, 0.f, 0.f, 1.f);
   CGContextSetLineWidth(ctx, 2.f);
   CGContextBeginPath(ctx);
   CGContextAddPath(ctx, path);
   CGContextDrawPath(ctx, kCGPathFillStroke);


   //Draw a rect, filled with pattern.
   CGColorSpaceRef colorSpace = CGColorSpaceCreatePattern(0);
   const float alpha = 1.f;
   CGContextSetFillColorSpace(ctx, colorSpace);
   CGContextSetFillPattern(ctx, fillPattern, &alpha);
   CGContextBeginPath(ctx);
   CGContextAddPath(ctx, path);
   CGContextDrawPath(ctx, kCGPathFill);

   CGColorSpaceRelease(colorSpace);  
}

//______________________________________________________________________________
- (void)dealloc
{
   if (path)
      CGPathRelease(path);

   [super dealloc];
}

//______________________________________________________________________________
- (void) setCol : (unsigned) newCol andRow : (unsigned) newRow andPattern : (CGPatternRef) pattern
{
   col = newCol;
   row = newRow;
   fillPattern = pattern;

   //Create a new path for selection rectangle and for shadow.
   if (path)
      CGPathRelease(path);
      
   path = CGPathCreateMutable();
   const CGAffineTransform transform = CGAffineTransformMakeTranslation(29.f, 29.f);
   CGPathMoveToPoint(path, &transform, col * 80.f, row * 26.7f);
   CGPathAddLineToPoint(path, &transform, col * 80.f, (row + 1) * 26.7f);
   CGPathAddLineToPoint(path, &transform, (col + 1) * 80.f, (row + 1) * 26.7f);
   CGPathAddLineToPoint(path, &transform, (col + 1) * 80.f, row * 26.7f);
   CGPathAddLineToPoint(path, &transform, col * 80.f, row * 26.7f);
   
   self.layer.shadowPath = path;
}

@end
