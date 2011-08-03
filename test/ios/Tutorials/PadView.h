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

class FontManagerWrapper;
class PainterWrapper;
class PadWrapper;

@class SelectionView;
@class PictHintView;
@class PictView;

@interface PadView : UIView {
   FontManagerWrapper *fontManager;
   PainterWrapper *painter;
   PadWrapper *pad;
   
   float scaleFactor;
   SelectionView * sv;
   
   BOOL processPan;
   BOOL processTap;
}

- (id) initWithFrame:(CGRect)frame forPad:(PadWrapper*)pad withFontManager:(FontManagerWrapper*)fm andPainter:(PainterWrapper*)painter;
- (void) dealloc;

- (void) drawRect:(CGRect)rect;
- (void) clearPad;

- (void) handlePanGesture : (UIPanGestureRecognizer *) panGesture;
- (void) handleTapGesture : (UITapGestureRecognizer *) tapGesture;

- (void) setSelectionView : (SelectionView *) selView;

- (void) setProcessPan : (BOOL) p;
- (void) setProcessTap : (BOOL) t;

@end
