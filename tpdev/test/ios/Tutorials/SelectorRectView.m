//
//  SelectorRectView.m
//  Tutorials
//
//  Created by Timur Pocheptsov on 8/9/11.
//  Copyright 2011 __MyCompanyName__. All rights reserved.
//

#import "SelectorRectView.h"


@implementation SelectorRectView

@synthesize glassImage;

//______________________________________________________________________________
- (id)initWithFrame:(CGRect)frame
{
   self = [super initWithFrame:frame];
   if (self) {
      // Initialization code
      self.glassImage = [UIImage imageNamed : @"fill_pattern_select_glass.png"];
   }

   return self;
}

//______________________________________________________________________________
- (void)drawRect:(CGRect)rect
{
   // Drawing code
   //Draw a rect, filled with pattern.
   [glassImage drawAtPoint:CGPointMake(29.f + col * 80.f, 29.f + row * 26.f)];
}

//______________________________________________________________________________
- (void)dealloc
{
   self.glassImage = nil;
   [super dealloc];
}

//______________________________________________________________________________
- (void) setCol : (unsigned) newCol andRow : (unsigned) newRow
{
   col = newCol;
   row = newRow;
}

@end
