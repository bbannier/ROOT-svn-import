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

@class ROOTObjectController;
@class SelectionView;

@interface PadView : UIView {
   ROOT_iOS::Pad *pad;

   UIPanGestureRecognizer *pan;
   UITapGestureRecognizer *singleTap;
   UITapGestureRecognizer *doubleTap;
   UILongPressGestureRecognizer *longPress;
   
   ROOTObjectController *controller;
   
   CGFloat currentScale;

   BOOL panActive;
   
   SelectionView *selectionView;
}

@property (nonatomic, assign) SelectionView *selectionView;

- (id) initWithFrame : (CGRect)frame controller : (ROOTObjectController *)c forPad : (ROOT_iOS::Pad*)pad;
- (void) dealloc;

- (void) drawRect : (CGRect)rect;
- (void) clearPad;

- (BOOL) pointOnSelectedObject : (CGPoint) pt;
- (void) addPanRecognizer;
- (void) removePanRecognizer;


- (void) handleSingleTap : (UITapGestureRecognizer*)tapGesture;
- (void) handleDoubleTap : (UITapGestureRecognizer*)tapGesture;
- (void) handlePan : (UIPanGestureRecognizer*)panGesture;
- (void) handleLongPressGesture : (UILongPressGestureRecognizer *)longPress;

@end
