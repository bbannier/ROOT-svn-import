//
//  SelectorRectView.h
//  Tutorials
//
//  Created by Timur Pocheptsov on 8/9/11.
//  Copyright 2011 __MyCompanyName__. All rights reserved.
//

#import <CoreGraphics/CGPattern.h>
#import <CoreGraphics/CGPath.h>

#import <UIKit/UIKit.h>

//
// Transparent view with a selector's "glass" rectangle
//

@interface SelectorRectView : UIView {
   unsigned col;
   unsigned row;

   CGMutablePathRef path; //Shadow path and selection rectangle.
   CGPatternRef fillPattern; //Current fill pattern.
}

- (void) setCol : (unsigned) col andRow : (unsigned) row andPattern : (CGPatternRef) pattern;

@end
