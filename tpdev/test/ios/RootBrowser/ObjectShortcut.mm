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

//C++ (ROOT) imports.
#import "TObject.h"

@implementation ObjectShortcut

@synthesize icon;

//____________________________________________________________________________________________________
+ (CGFloat) iconWidth
{
   return 150.f;
}

//____________________________________________________________________________________________________
+ (CGFloat) iconHeight
{
   return 150.f;
}

//____________________________________________________________________________________________________
+ (CGFloat) textHeight
{
   return 100.f;
}

//____________________________________________________________________________________________________
+ (CGRect) defaultRect
{
   return CGRectMake(0.f, 0.f, [ObjectShortcut iconWidth], [ObjectShortcut iconHeight] + [ObjectShortcut textHeight]);
}

//____________________________________________________________________________________________________
- (id)initWithFrame:(CGRect)frame controller : (FileContentController *)c object : (TObject *)object thumbnail : (UIImage *)thumbnail
{
   self = [super initWithFrame:frame];
   if (self) {
      controller = c;
      self.icon = thumbnail;
      rootObject = object;

      self.layer.shadowColor = [UIColor blackColor].CGColor;
      self.layer.shadowOpacity = 0.3;
      self.layer.shadowOffset = CGSizeMake(10.f, 10.f);
      frame.origin = CGPointZero;
      frame.size.height = [ObjectShortcut iconHeight];
      self.layer.shadowPath = [UIBezierPath bezierPathWithRect : frame].CGPath;
      
      self.opaque = NO;
      
      UITapGestureRecognizer *tap = [[UITapGestureRecognizer alloc] initWithTarget:self action:@selector(handleTap)];
      [self addGestureRecognizer : tap];
      [tap release];
   }

   return self;
}

//____________________________________________________________________________________________________
- (void)drawRect:(CGRect)rect
{
   [icon drawAtPoint:CGPointZero];
   
   CGContextRef ctx = UIGraphicsGetCurrentContext();
   
   CGContextSetRGBFillColor(ctx, 1.f, 1.f, 1.f, 1.f);
   
   NSString * objectName = [NSString stringWithFormat:@"%s", rootObject->GetName()];
   const CGRect textRect = CGRectMake(0.f, [ObjectShortcut iconHeight], [ObjectShortcut iconWidth], [ObjectShortcut textHeight]);
   [objectName drawInRect : textRect withFont : [UIFont systemFontOfSize : 16] lineBreakMode : UILineBreakModeWordWrap alignment : UITextAlignmentCenter];
}

//____________________________________________________________________________________________________
- (void)dealloc
{
   self.icon = nil;
   [super dealloc];
}

//____________________________________________________________________________________________________
- (void) handleTap
{
   [controller selectObjectFromFile : rootObject];
}

@end
