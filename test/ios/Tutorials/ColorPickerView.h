//
//  ColorPickerView.h
//  Tutorials
//
//  Created by Timur Pocheptsov on 8/7/11.
//  Copyright 2011 __MyCompanyName__. All rights reserved.
//

#import <UIKit/UIKit.h>

#import <CoreGraphics/CoreGraphics.h>

@class SelectorArrowView;
@class NSTimer;

@interface ColorPickerView : UIView {
   UIImage *colorWheelImage;
   //
   SelectorArrowView *selectorView;
   //
   CGPoint wheelCenter;
   double radius;
   
   double rotationAngle;
   double dAngle;
   
   unsigned currentSector;
   unsigned nextSector;
   
   NSTimer *timer;
   unsigned nFrame;
   BOOL activeAnimation;
}

- (void) setupColorPicker;

@property (nonatomic, retain) UIImage *colorWheelImage;
@property (nonatomic, retain) SelectorArrowView *selectorView;
@property (nonatomic, retain) NSTimer *timer;

@end
