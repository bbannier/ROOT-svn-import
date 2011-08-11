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
- (void)dealloc
{
   [super dealloc];
}

//______________________________________________________________________________
- (void) setFillPattern : (CGPatternRef) pattern
{
   fillPattern = pattern;
}

//______________________________________________________________________________
- (void)drawRect:(CGRect)rect
{
   CGContextRef ctx = UIGraphicsGetCurrentContext();

   //Fill view with pattern.
   CGColorSpaceRef colorSpace = CGColorSpaceCreatePattern(0);
   const float alpha = 1.f;
   
   CGContextSetFillColorSpace(ctx, colorSpace);
   CGContextSetFillPattern(ctx, fillPattern, &alpha);
   CGContextFillRect(ctx, rect);

   CGColorSpaceRelease(colorSpace);
}

@end
