//
//  PatternPickerView.h
//  Tutorials
//
//  Created by Timur Pocheptsov on 8/9/11.
//  Copyright 2011 __MyCompanyName__. All rights reserved.
//
#import <CoreGraphics/CGPattern.h>

#import <UIKit/UIKit.h>

#include "FillPatterns.h"

@class SelectorRectView;


@interface PatternPickerView : UIView {
   CGPatternRef patterns[ROOT_iOS::GraphicUtils::kPredefinedFillPatterns];

   UIImage *backgroundImage;
   
   SelectorRectView *selectorView;
}

@property (nonatomic, retain) UIImage *backgroundImage;

- (void) setupPatternPicker;

@end
