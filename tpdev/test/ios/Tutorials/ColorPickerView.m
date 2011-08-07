//
//  ColorPickerView.m
//  Tutorials
//
//  Created by Timur Pocheptsov on 8/7/11.
//  Copyright 2011 __MyCompanyName__. All rights reserved.
//

#import <CoreGraphics/CoreGraphics.h>
#import <CoreGraphics/CGContext.h>
#import <QuartzCore/QuartzCore.h>

#import <math.h>

#import "SelectorArrowView.h"
#import "ColorPickerView.h"

static double inline deg_to_rad(double angle)
{
   return angle * M_PI / 180.;
}

@implementation ColorPickerView

@synthesize colorWheelImage;
@synthesize selectorView;

- (void) onTimer 
{
//   NSLog(@"timer");
   if (nFrame + 1 < 50) {
      ++nFrame;
      rotationAngle += dAngle;
   } else {
      [timer invalidate];
      currentSector = nextSector;
   }
  
   [selectorView setAngle : rotationAngle];
     
   [selectorView setNeedsDisplay];
}

- (void) handleTap : (UITapGestureRecognizer *) tapGesture
{
   const CGPoint pt = [tapGesture locationInView : self];
   const double x = pt.x - wheelCenter.x;
   const double y = self.bounds.size.height - pt.y - wheelCenter.y;
   if (x * x + y * y < radius * radius) {
      //Tap is inside a color wheel.
      double angle = atan(y / (x + 0.01));
      
      if (x < 0.) {
         angle = M_PI + angle;
      } else if (y < 0)
         angle += 2 * M_PI;

      nextSector = (unsigned)((angle * 180 / M_PI) / 22.5);
      
      if (nextSector > currentSector) {
         dAngle = (nextSector - currentSector) * deg_to_rad(-22.5) / 49;
      } else {
         dAngle = - deg_to_rad(-22.5) * (currentSector - nextSector) / 49;
      }
      
      nFrame = 0;
      timer = [NSTimer scheduledTimerWithTimeInterval:0.25 / 50 target : self selector:@selector(onTimer) userInfo:nil repeats:YES];
   }
}

- (id)initWithFrame:(CGRect)frame
{
   self = [super initWithFrame:frame];

   if (self) {
      // Initialization code
      [self setupColorPicker];
   }
   
   return self;
}

// Only override drawRect: if you perform custom drawing.
// An empty implementation adversely affects performance during animation.
- (void)drawRect:(CGRect)rect
{
    // Drawing code
   if (!colorWheelImage)
      [self setupColorPicker];

   [colorWheelImage drawInRect:rect];
}

- (void) setupColorPicker
{
   //color wheel image is square (and view for this control is square).
   self.colorWheelImage = [UIImage imageNamed:@"color_wheel.png"];

   const CGSize size = [colorWheelImage size];
   
   wheelCenter.x = size.width / 2;
   wheelCenter.y = size.height / 2;
   
   rotationAngle = 0.f;
   currentSector = 0;
   nextSector = 0;
   
   //This is the ratio from my "color_wheel.png".
   radius = 0.84 * (size.width / 2);

   self.selectorView = [[SelectorArrowView alloc] initWithFrame : CGRectMake(0.f, 0.f, size.width, size.height)];
   [self addSubview : selectorView];
   selectorView.opaque = NO;

   [selectorView setCenter : wheelCenter];
   [selectorView setAngle : 0.f];
   
   selectorView.layer.shadowColor = [UIColor blackColor].CGColor;
   selectorView.layer.shadowOpacity = 0.9f;
   selectorView.layer.shadowOffset = CGSizeMake(10.f, 10.f);
   
   UITapGestureRecognizer *tapGesture = [[UITapGestureRecognizer alloc] initWithTarget: self action : @selector(handleTap:)];
   [self addGestureRecognizer : tapGesture];
   [tapGesture release];
}

- (void)dealloc
{
   self.colorWheelImage = nil;
   self.selectorView = nil;
   [super dealloc];
}

@end
