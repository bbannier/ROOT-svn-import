//
//  PatternCell.m
//  Tutorials
//
//  Created by Timur Pocheptsov on 8/11/11.
//  Copyright 2011 __MyCompanyName__. All rights reserved.
//

#import <CoreGraphics/CGContext.h>

#import "PatternCell.h"


@implementation PatternCell

- (id)initWithFrame:(CGRect)frame
{
    self = [super initWithFrame:frame];
    if (self) {
        // Initialization code
    }
    return self;
}

- (void) setFillPattern : (CGPatternRef) pattern
{
   fillPattern = pattern;
}

- (void) setDarkBackground
{
   darkBackground = YES;
}

- (void)drawRect:(CGRect)rect
{
   CGContextRef ctx = UIGraphicsGetCurrentContext();

  /* if (darkBackground)
      CGContextSetRGBFillColor(ctx, 0.7f, 0.7f, 0.7f, 1.f);
   else
      CGContextSetRGBFillColor(ctx, 1.f, 1.f, 1.f, 1.f);

   CGContextFillRect(ctx, rect);*/

   //Now fill view with pattern.
   CGColorSpaceRef colorSpace = CGColorSpaceCreatePattern(0);
   const float alpha = 1.f;
   
   CGContextSetFillColorSpace(ctx, colorSpace);
//   CGContextSetRGBFillColor(ctx, 0.f, 0.f, 0.f, 1.f);
   CGContextSetFillPattern(ctx, fillPattern, &alpha);
   CGContextFillRect(ctx, rect);

   CGColorSpaceRelease(colorSpace);
}

- (void)dealloc
{
   [super dealloc];
}

@end
