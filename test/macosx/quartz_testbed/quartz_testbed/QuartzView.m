//
//  QuartzView.m
//  quartz_testbed
//
//  Created by Timur Pocheptsov on 12/1/11.
//  Copyright (c) 2011 __MyCompanyName__. All rights reserved.
//

#import "QuartzView.h"

@implementation QuartzView

- (id)initWithFrame:(NSRect)frame
{
   if (self = [super initWithFrame : frame]) {
      // Initialization code here.
   }
    
   return self;
}

- (void) drawRect : (NSRect)dirtyRect
{
   // Drawing code here.
   NSGraphicsContext *nsContext = [NSGraphicsContext currentContext];
   CGContextRef cgContext = (CGContextRef)[nsContext graphicsPort];
   
   CGContextSetRGBFillColor(cgContext, 1.f, 0.3f, 0.f, 1.f);
   CGContextFillRect(cgContext, dirtyRect);
}

@end
