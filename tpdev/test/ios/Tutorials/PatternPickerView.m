//
//  PatternPickerView.m
//  Tutorials
//
//  Created by Timur Pocheptsov on 8/9/11.
//  Copyright 2011 __MyCompanyName__. All rights reserved.
//

#import "PatternPickerView.h"


@implementation PatternPickerView

@synthesize backgroundImage;

- (id) initWithFrame : (CGRect)frame
{
   self = [super initWithFrame : frame];
    
   if (self) {
      [self setupPatternPicker];
   }
    
   return self;
}


- (void)drawRect:(CGRect)rect
{
   // Drawing code
   if (!backgroundImage)
      [self setupPatternPicker];
   
   [backgroundImage drawAtPoint : CGPointZero];
}


- (void)dealloc
{
   self.backgroundImage = nil;
   [super dealloc];
}

- (void) setupPatternPicker
{
   self.backgroundImage = [UIImage imageNamed : @"fill_pattern_select.png"];
}

@end
