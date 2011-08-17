//
//  ColorCell.m
//  editors
//
//  Created by Timur Pocheptsov on 8/16/11.
//  Copyright 2011 __MyCompanyName__. All rights reserved.
//

#import "ColorCell.h"


#import "ColorCell.h"


@implementation ColorCell

//______________________________________________________________________________
+ (CGFloat) cellAlpha 
{
   return 0.8f;
}

//______________________________________________________________________________
- (id)initWithFrame:(CGRect)frame
{
   self = [super initWithFrame:frame];
   
   if (self)
      self.backgroundColor = [UIColor clearColor];

   return self;
}

//______________________________________________________________________________
- (void)dealloc
{
    [super dealloc];
}

//______________________________________________________________________________
- (void) setRGB : (const double *) newRgb
{
   rgb[0] = newRgb[0];
   rgb[1] = newRgb[1];
   rgb[2] = newRgb[2];
}

//______________________________________________________________________________
- (void)drawRect:(CGRect)rect
{
   CGContextRef ctx = UIGraphicsGetCurrentContext();
   if (!ctx) {
      //Log error: ctx is nil.
      return;
   }

   CGContextSetRGBFillColor(ctx, rgb[0], rgb[1], rgb[2], [ColorCell cellAlpha]);
   CGContextFillRect(ctx, rect);
}

@end
