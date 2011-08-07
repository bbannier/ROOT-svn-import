//
//  SelectorView.h
//  Tutorials
//
//  Created by Timur Pocheptsov on 8/7/11.
//  Copyright 2011 __MyCompanyName__. All rights reserved.
//

#import <UIKit/UIKit.h>

//
//Small class to draw semi-transparent arrow 
//above the selected color.
//

@interface SelectorArrowView : UIView {
   UIImage *arrowImage;
   
   CGPoint center;
   double angle;
}

@property (nonatomic, retain) UIImage *arrowImage;

- (id) initWithFrame : (CGRect)frame;
- (void) dealloc;

- (void) drawRect : (CGRect)rect;

- (void) setCenter : (CGPoint) center;
- (void) setAngle : (double) angle;

@end
