//
//  SelectorView.m
//  Tutorials
//
//  Created by Timur Pocheptsov on 8/7/11.
//  Copyright 2011 __MyCompanyName__. All rights reserved.
//
#import <math.h>

#import <QuartzCore/QuartzCore.h>

#import "SelectorArrowView.h"


static const double sectorAngle = 22.5 * M_PI / 180;

@implementation SelectorArrowView


//______________________________________________________________________________
- (id)initWithFrame : (CGRect)frame
{
   self = [super initWithFrame : frame];
    
   if (self) {
      // Initialization code
      sectorColor[0] = 1.f;
      sectorColor[1] = 1.f;
      sectorColor[2] = 1.f;
      
      arrowPath = 0;
   }
   
   return self;
}

//______________________________________________________________________________
- (void)drawRect:(CGRect)rect
{
   // Drawing code

   CGContextRef ctx = UIGraphicsGetCurrentContext();

   CGContextSetRGBFillColor(ctx, sectorColor[0], sectorColor[1], sectorColor[2], 1.f);
   CGContextSetRGBStrokeColor(ctx, 0.f, 0.f, 0.f, 1.f);
   CGContextSetLineWidth(ctx, 2.5f);
   
   CGContextBeginPath(ctx);
   CGContextAddPath(ctx, arrowPath);
   CGContextDrawPath(ctx, kCGPathFillStroke);
}

//______________________________________________________________________________
- (void)dealloc
{
   if (arrowPath)
      CGPathRelease(arrowPath);

   [super dealloc];
}

//______________________________________________________________________________
- (void) setCenter : (CGPoint) c andRadius : (double)r
{
   center = c;
   radius = r;
}

//______________________________________________________________________________
- (void) setColor : (const double *)rgb
{
   for (unsigned i = 0; i < 3; ++i)
      sectorColor[i] = rgb[i];
}

//______________________________________________________________________________
- (void) setAngle : (double) a
{
   angle = a;
   
   CGAffineTransform transform = CGAffineTransformMakeTranslation(center.x, center.y);
   transform = CGAffineTransformRotate(transform, angle);
      
   if (arrowPath)
      CGPathRelease(arrowPath);
   
   arrowPath = CGPathCreateMutable();
   CGPathMoveToPoint(arrowPath, &transform, 0.f, 0.f);
   CGPathAddLineToPoint(arrowPath, &transform, radius, 0.f);
   CGPathAddLineToPoint(arrowPath, &transform, radius * cos(sectorAngle), -radius * sin(sectorAngle));
   CGPathAddLineToPoint(arrowPath, &transform, 0.f, 0.f);   
   self.layer.shadowPath = arrowPath;
}

@end
