//
//  PatternPickerView.m
//  Tutorials
//
//  Created by Timur Pocheptsov on 8/9/11.
//  Copyright 2011 __MyCompanyName__. All rights reserved.
//

#import <CoreGraphics/CGContext.h>
#import <QuartzCore/QuartzCore.h>

#import "PatternPickerView.h"
#import "SelectorRectView.h"


@implementation PatternPickerView

@synthesize backgroundImage;

//______________________________________________________________________________
- (id) initWithFrame : (CGRect)frame
{
   self = [super initWithFrame : frame];
    
   if (self) {
      [self setupPatternPicker];
   }
    
   return self;
}

//______________________________________________________________________________
- (void)drawRect:(CGRect)rect
{
   // Drawing code
   if (!backgroundImage)
      [self setupPatternPicker];
   
   [backgroundImage drawAtPoint : CGPointZero];
   
   CGContextRef ctx = UIGraphicsGetCurrentContext();

   CGColorSpaceRef colorSpace = CGColorSpaceCreatePattern(0);
   const float alpha = 1.f;
   
   //Draw fill patterns here.
   for (unsigned col = 0, nCell = 0; col < 3; ++col) {
      for (unsigned row = 0; row < 9; ++row, ++nCell) {
         if (nCell >= ROOT_iOS::GraphicUtils::kPredefinedFillPatterns)
            break;

         CGContextSetFillColorSpace(ctx, colorSpace);
         CGContextSetFillPattern(ctx, patterns[nCell], &alpha);

         //Draw a rect, filled with pattern.
         const CGRect fillRect = CGRectMake(29.f + col * 80.f, 29.f + row * 26.f, 80.f, 26.f);
         CGContextFillRect(ctx, fillRect);
      }
      
      if (nCell >= ROOT_iOS::GraphicUtils::kPredefinedFillPatterns)
         break;
   }

   CGColorSpaceRelease(colorSpace);
}

//______________________________________________________________________________
- (void)dealloc
{
   self.backgroundImage = nil;
   
   for (unsigned i = 0; i < ROOT_iOS::GraphicUtils::kPredefinedFillPatterns; ++i) {
      CGPatternRelease(patterns[i]);
   }
   
   [super dealloc];
}

//______________________________________________________________________________
- (void) handleTap : (UITapGestureRecognizer *) tapGesture
{
   CGPoint location = [tapGesture locationInView : self];

   if (location.x < 29 || location.x > 269)
      return;
   if (location.y < 29 || location.y >= 263)
      return;
   

   NSLog(@"%g %g", location.x, location.y);
   
   location.x -= 29;
   location.y -= 29;
   
   const unsigned newCol = location.x / 80;
   const unsigned newRow = location.y / 26;
   
   if (newCol * 9 + newRow >= ROOT_iOS::GraphicUtils::kPredefinedFillPatterns)
      return;
   
   [selectorView setCol : newCol andRow : newRow];
   [selectorView setNeedsDisplay];
}

//______________________________________________________________________________
- (void) setupPatternPicker
{
   self.backgroundImage = [UIImage imageNamed : @"pattern_selector_background.png"];

   //Prepare fill patterns:
   for (unsigned i = 0; i < ROOT_iOS::GraphicUtils::kPredefinedFillPatterns; ++i) {
      patterns[i] = ROOT_iOS::GraphicUtils::gPatternGenerators[i]();
      CGPatternRetain(patterns[i]);
   }
   
   selectorView = [[SelectorRectView alloc] initWithFrame : CGRectMake(0.f, 0.f, 300.f, 300.f)];
   selectorView.opaque = NO;
   
   selectorView.layer.shadowColor = [UIColor blackColor].CGColor;
   selectorView.layer.shadowOpacity = 0.9f;
   selectorView.layer.shadowOffset = CGSizeMake(5.f, 5.f);

   
   [self addSubview : selectorView];
   
   UITapGestureRecognizer * tap = [[UITapGestureRecognizer alloc] initWithTarget : self action : @selector(handleTap:)];
   [self addGestureRecognizer : tap];
   [tap release];
}

@end
