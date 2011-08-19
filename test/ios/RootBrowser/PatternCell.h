//
//  PatternCell.h
//  editors
//
//  Created by Timur Pocheptsov on 8/16/11.
//  Copyright 2011 __MyCompanyName__. All rights reserved.
//

#import <UIKit/UIKit.h>

@interface PatternCell : UIView {
   unsigned patternIndex;
   BOOL solid;
}

- (id) initWithFrame : (CGRect) frame andPattern : (unsigned) index;
- (void) dealloc;

- (void) setAsSolid;
- (void) drawRect : (CGRect) rect;

@end
