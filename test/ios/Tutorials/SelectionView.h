//
//  SelectionView.h
//  Tutorials
//
//  Created by Timur Pocheptsov on 7/22/11.
//  Copyright 2011 __MyCompanyName__. All rights reserved.
//

#import <UIKit/UIKit.h>

class PainterWrapper;
class PadWrapper;

@class PadView;

@interface SelectionView : UIView {
   BOOL showRotation;
   int ev;
   int px;
   int py;
   PadWrapper *pad;
   PainterWrapper *painter;
   PadView *view;
}

- (void) setShowRotation : (BOOL) show;
- (void) setEvent : (int) ev atX : (int) x andY : (int) y;
- (void) setPad : (PadWrapper *)pad andPainter : (PainterWrapper *)painter;

@end
