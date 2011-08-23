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

@interface PadView : UIView {
   ROOT_iOS::Pad *pad;

//   float scaleFactor;
//   SelectionView *selectionView;
   
//   BOOL processPan;
//   BOOL processTap;
}

- (id) initWithFrame : (CGRect)frame forPad : (ROOT_iOS::Pad*)pad;
- (void) dealloc;

- (void) drawRect : (CGRect)rect;
- (void) clearPad;

//- (void) handlePanGesture : (UIPanGestureRecognizer *)panGesture;
//- (void) handleTapGesture : (UITapGestureRecognizer *)tapGesture;

//- (void) setSelectionView : (SelectionView *) sv;

//- (void) setProcessPan : (BOOL)p;
//- (void) setProcessTap : (BOOL)t;


@end
