//
//  LineStyleCell.m
//  root_browser
//
//  Created by Timur Pocheptsov on 8/31/11.
//  Copyright 2011 __MyCompanyName__. All rights reserved.
//

#import <CoreGraphics/CGContext.h>

#import "LineStyleCell.h"

//C++ (ROOT) imports.
#import "IOSLineStyles.h"

@implementation LineStyleCell

//_________________________________________________________________
- (id) initWithFrame : (CGRect)frame lineStyle : (unsigned) style
{
   self = [super initWithFrame:frame];

   if (self) {
      lineStyle = style;
      backgroundImage = [UIImage imageNamed : @"line_cell.png"];
      [backgroundImage retain];
   }

   return self;
}

//_________________________________________________________________
- (void)drawRect:(CGRect)rect
{
   CGContextRef ctx = UIGraphicsGetCurrentContext();
   
   [backgroundImage drawInRect : rect];

   if (lineStyle > 1 && lineStyle <= 10)
      CGContextSetLineDash(ctx, 0., ROOT_iOS::GraphicUtils::dashLinePatterns[lineStyle - 1], ROOT_iOS::GraphicUtils::linePatternLengths[lineStyle - 1]);
   else
      CGContextSetLineDash(ctx, 0., 0, 0);
   
   CGContextSetLineWidth(ctx, 2.f);
   
   CGContextBeginPath(ctx);
   CGContextMoveToPoint(ctx, 10.f, rect.size.height / 2);
   CGContextAddLineToPoint(ctx, rect.size.width - 10, rect.size.height / 2);
   CGContextStrokePath(ctx);
}

//_________________________________________________________________
- (void)dealloc
{
   [backgroundImage release];
   [super dealloc];
}

@end
