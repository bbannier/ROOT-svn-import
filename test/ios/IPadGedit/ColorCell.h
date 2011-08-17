//
//  ColorCell.h
//  editors
//
//  Created by Timur Pocheptsov on 8/16/11.
//  Copyright 2011 __MyCompanyName__. All rights reserved.
//

#import <UIKit/UIKit.h>

@interface ColorCell : UIView {
   float rgb[3];
}

- (id) initWithFrame : (CGRect) frame;
- (void) dealloc;

- (void) setRGB : (const double *) rgb;
- (void) drawRect : (CGRect) rect;

+ (CGFloat) cellAlpha;

@end
