//
//  SelectorView.m
//  Tutorials
//
//  Created by Timur Pocheptsov on 8/7/11.
//  Copyright 2011 __MyCompanyName__. All rights reserved.
//

#import "SelectorArrowView.h"


@implementation SelectorArrowView

@synthesize arrowImage;

- (id)initWithFrame : (CGRect)frame
{
   self = [super initWithFrame : frame];
    
   if (self) {
      // Initialization code
      self.arrowImage = [UIImage imageNamed:@"color_selector.png"];
   }
   
   return self;
}

- (void)drawRect:(CGRect)rect
{
    // Drawing code

   CGContextRef ctx = UIGraphicsGetCurrentContext();

   CGContextTranslateCTM(ctx, center.x, center.y);
   CGContextRotateCTM(ctx, angle);
   CGContextTranslateCTM(ctx, 0.f, -[arrowImage size].height);
   
   [arrowImage drawAtPoint:CGPointZero];
}

- (void)dealloc
{
   self.arrowImage = nil;
   [super dealloc];
}

- (void) setCenter : (CGPoint) c
{
   center = c;
}

- (void) setAngle : (double) a
{
   angle = a;
}

@end
