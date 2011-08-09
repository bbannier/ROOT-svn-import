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

//______________________________________________________________________________
- (id)initWithFrame : (CGRect)frame
{
   self = [super initWithFrame : frame];
    
   if (self) {
      // Initialization code
      self.arrowImage = [UIImage imageNamed:@"color_selector.png"];
   }
   
   return self;
}

//______________________________________________________________________________
- (void)drawRect:(CGRect)rect
{
    // Drawing code

   CGContextRef ctx = UIGraphicsGetCurrentContext();

   CGContextTranslateCTM(ctx, center.x, center.y);
   CGContextRotateCTM(ctx, angle);
   CGContextTranslateCTM(ctx, 0.f, -[arrowImage size].height);
   
   [arrowImage drawAtPoint:CGPointZero];
}

//______________________________________________________________________________
- (void)dealloc
{
   self.arrowImage = nil;
   [super dealloc];
}

//______________________________________________________________________________
- (void) setCenter : (CGPoint) c
{
   center = c;
}

//______________________________________________________________________________
- (void) setAngle : (double) a
{
   angle = a;
}

@end
