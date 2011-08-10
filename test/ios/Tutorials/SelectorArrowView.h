//
//  SelectorView.h
//  Tutorials
//
//  Created by Timur Pocheptsov on 8/7/11.
//  Copyright 2011 __MyCompanyName__. All rights reserved.
//

#import <CoreGraphics/CGPath.h>

#import <UIKit/UIKit.h>

//
//Small class to draw semi-transparent arrow 
//above the selected color.
//

@interface SelectorArrowView : UIView {
   CGPoint center;
   double angle;
   double radius;
   
   double sectorColor[3];//rgb
   
   CGMutablePathRef arrowPath;
}

- (id) initWithFrame : (CGRect)frame;
- (void) dealloc;

- (void) drawRect : (CGRect)rect;

- (void) setCenter : (CGPoint) center andRadius : (double) radius;
- (void) setAngle : (double) angle;
- (void) setColor : (const double *)rgb;


@end
