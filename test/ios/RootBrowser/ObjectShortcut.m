//
//  ObjectShortcut.m
//  root_browser
//
//  Created by Timur Pocheptsov on 8/19/11.
//  Copyright 2011 __MyCompanyName__. All rights reserved.
//

#import <CoreGraphics/CGContext.h>
#import <QuartzCore/QuartzCore.h>

#import "FileContentController.h"
#import "ObjectShortcut.h"

@implementation ObjectShortcut

@synthesize icon;
@synthesize objectName;

+ (CGFloat) iconWidth
{
   return 150.f;
}

+ (CGFloat) iconHeight
{
   return 150.f;
}

+ (CGFloat) textHeight
{
   return [ObjectShortcut iconHeight] + 100.f;
}

+ (CGRect) defaultRect
{
   return CGRectMake(0.f, 0.f, [ObjectShortcut iconWidth], [ObjectShortcut iconHeight] + [ObjectShortcut textHeight]);
}

- (id)initWithFrame:(CGRect)frame controller : (FileContentController *)c objectName : (NSString *)name
{
   self = [super initWithFrame:frame];
   if (self) {
      controller = c;
   
      self.icon = [UIImage imageNamed:@"lego_icon.png"];
      self.objectName = name;
      
      self.layer.shadowColor = [UIColor blackColor].CGColor;
      self.layer.shadowOpacity = 0.3;
      self.layer.shadowOffset = CGSizeMake(10.f, 10.f);
      frame.origin = CGPointZero;
      frame.size.height = [ObjectShortcut iconHeight];
      self.layer.shadowPath = [UIBezierPath bezierPathWithRect:frame].CGPath;

      self.opaque = NO;
      
      UITapGestureRecognizer *tap = [[UITapGestureRecognizer alloc] initWithTarget:self action:@selector(handleTap)];
      [self addGestureRecognizer : tap];
      [tap release];
   }

   return self;
}

- (void)drawRect:(CGRect)rect
{
   [icon drawAtPoint:CGPointZero];
   
   CGContextRef ctx = UIGraphicsGetCurrentContext();
   
   CGContextSetRGBFillColor(ctx, 1.f, 1.f, 1.f, 1.f);
   const CGRect textRect = CGRectMake(0.f, [ObjectShortcut iconHeight], [ObjectShortcut iconWidth], [ObjectShortcut textHeight]);
   [objectName drawInRect : textRect withFont : [UIFont systemFontOfSize : 16] lineBreakMode : UILineBreakModeWordWrap alignment : UITextAlignmentCenter];
}

- (void)dealloc
{
   self.icon = nil;
   self.objectName = nil;
   [super dealloc];
}

- (void) handleTap
{
   [controller selectFromFile : self];
}

@end
