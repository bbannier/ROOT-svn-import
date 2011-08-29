//
//  PadView.h
//  Tutorials
//
//  Created by Timur Pocheptsov on 7/8/11.
//  Copyright 2011 __MyCompanyName__. All rights reserved.
//

#import <UIKit/UIKit.h>

///////////////////////////////////////////////////////////
//  Custom view, subview for a detail view.
//  Delegates all graphics to C++ code.
///////////////////////////////////////////////////////////

namespace ROOT_iOS {

class Pad;

}

//@class SelectionView;
@class ROOTObjectController;

@interface PadView : UIView {
   ROOT_iOS::Pad *pad;

   UIPanGestureRecognizer *pan;
   UITapGestureRecognizer *singleTap;
   UITapGestureRecognizer *doubleTap;
   UILongPressGestureRecognizer *longPress;
   
   ROOTObjectController *controller;

   BOOL panActive;
}

- (id) initWithFrame : (CGRect)frame controller : (ROOTObjectController *)c forPad : (ROOT_iOS::Pad*)pad;
- (void) dealloc;

- (void) drawRect : (CGRect)rect;
- (void) clearPad;

- (BOOL) pointOnSelectedObject : (CGPoint) pt;
- (void) addPanRecognizer;
- (void) removePanRecognizer;

//- (void) turnOnEditMode;
//- (void) turnOffEditoMode;

- (void) handleSingleTap : (UITapGestureRecognizer*)tapGesture;
- (void) handleDoubleTap : (UITapGestureRecognizer*)tapGesture;
- (void) handlePan : (UIPanGestureRecognizer*)panGesture;
- (void) handleLongPressGesture : (UILongPressGestureRecognizer *)longPress;

//- (void) setSelectionView : (SelectionView *) sv;

@end
