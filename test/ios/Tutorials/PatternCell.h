//
//  PatternCell.h
//  Tutorials
//
//  Created by Timur Pocheptsov on 8/11/11.
//  Copyright 2011 __MyCompanyName__. All rights reserved.
//

#import <CoreGraphics/CGPattern.h>
#import <UIKit/UIKit.h>


@interface PatternCell : UIView {
   CGPatternRef fillPattern;
   BOOL darkBackground;
}

- (id) initWithFrame : (CGRect) frame;
- (void) dealloc;

- (void) setFillPattern : (CGPatternRef) pattern;
- (void) setDarkBackground;
- (void) drawRect : (CGRect) rect;

@end
