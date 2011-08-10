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

static const double predefinedFillColors[16][3] = 
{
{1., 1., 1.},
{0., 0., 0.},
{251 / 255., 0., 24 / 255.},
{40 / 255., 253 / 255., 44 / 255.},
{31 / 255., 29 / 255., 251 / 255.},
{253 / 255., 254 / 255., 52 / 255.},
{253 / 255., 29 / 255., 252 / 255.},
{53 / 255., 1., 254 / 255.},
{94 / 255., 211 / 255., 90 / 255.},
{92 / 255., 87 / 255., 214 / 255.},
{135 / 255., 194 / 255., 164 / 255.},
{127 / 255., 154 / 255., 207 / 255.},
{211 / 255., 206 / 255., 138 / 255.},
{220 / 255., 185 / 255., 138 / 255.},
{209 / 255., 89 / 255., 86 / 255.},
{147 / 255., 29 / 255., 251 / 255.}
};

@implementation ColorPickerView

@synthesize colorWheelImage;
@synthesize selectorView;
@synthesize timer;

//______________________________________________________________________________
- (void) onTimer 
{
   if (nFrame + 1 < 50) {
      ++nFrame;
      rotationAngle += dAngle;
   } else {
      activeAnimation = NO;
      [timer invalidate];
      self.timer = nil;
      currentSector = nextSector;
      [selectorView setColor : predefinedFillColors[currentSector]];
   }
  
   [selectorView setAngle : rotationAngle];

   [selectorView setNeedsDisplay];
}

//______________________________________________________________________________
- (void) handleTap : (UITapGestureRecognizer *) tapGesture
{
   if (activeAnimation)
      return;

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
      activeAnimation = YES;
      self.timer = [NSTimer scheduledTimerWithTimeInterval:0.25 / 50 target : self selector:@selector(onTimer) userInfo:nil repeats:YES];
   }
}

//______________________________________________________________________________
- (id)initWithFrame:(CGRect)frame
{
   self = [super initWithFrame:frame];

   if (self) {
      // Initialization code
      [self setupColorPicker];
   }
   
   return self;
}

//______________________________________________________________________________
- (void)drawRect:(CGRect)rect
{
    // Drawing code
   if (!colorWheelImage)
      [self setupColorPicker];

   [colorWheelImage drawInRect:rect];
}

//______________________________________________________________________________
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

   [selectorView setCenter : wheelCenter andRadius : radius];
   [selectorView setAngle : 0.f];
   [selectorView setColor : predefinedFillColors[currentSector]];

   selectorView.layer.shadowColor = [UIColor blackColor].CGColor;
   selectorView.layer.shadowOpacity = 0.9f;
   selectorView.layer.shadowOffset = CGSizeMake(10.f, 10.f);

   UITapGestureRecognizer *tapGesture = [[UITapGestureRecognizer alloc] initWithTarget: self action : @selector(handleTap:)];
   [self addGestureRecognizer : tapGesture];
   [tapGesture release];
}

//______________________________________________________________________________
- (void)dealloc
{
   self.colorWheelImage = nil;
   self.selectorView = nil;
   [super dealloc];
}

@end
