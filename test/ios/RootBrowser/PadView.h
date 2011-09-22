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
//  This is view for pad in "editable state".
//  TODO: find better class name.
///////////////////////////////////////////////////////////

namespace ROOT_iOS {

class Pad;

}

@class ROOTObjectController;
@class SelectionView;

@interface PadView : UIView {
   ROOT_iOS::Pad *pad;
   
   ROOTObjectController *controller;
   
   CGFloat currentScale;

   BOOL panActive;
   
   SelectionView *selectionView;
   
   CGPoint tapPt;
   BOOL processSecondTap;
}

@property (nonatomic, assign) SelectionView *selectionView;

- (id) initWithFrame : (CGRect)frame controller : (ROOTObjectController *)c forPad : (ROOT_iOS::Pad*)pad;
- (void) dealloc;

- (void) setPad : (ROOT_iOS::Pad *)newPad;
- (void) drawRect : (CGRect)rect;
- (void) clearPad;

- (BOOL) pointOnSelectedObject : (CGPoint) pt;
- (void) addPanRecognizer;
- (void) removePanRecognizer;

- (void) handleSingleTap;
- (void) handleDoubleTap;

@end
