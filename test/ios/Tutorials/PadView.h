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

class FontManager;
class Painter;
class Pad;

}

@class SelectionView;
@class PictHintView;
@class PictView;

@interface PadView : UIView {
   ROOT_iOS::FontManager *fontManager;
   ROOT_iOS::Painter *painter;
   ROOT_iOS::Pad *pad;

   float scaleFactor;
   SelectionView *sv;
   
   BOOL processPan;
   BOOL processTap;
}

- (id) initWithFrame : (CGRect)frame forPad : (ROOT_iOS::Pad*)pad withFontManager : (ROOT_iOS::FontManager*)fm andPainter : (ROOT_iOS::Painter*)painter;
- (void) dealloc;

- (void) drawRect:(CGRect)rect;
- (void) clearPad;

- (void) handlePanGesture : (UIPanGestureRecognizer *)panGesture;
- (void) handleTapGesture : (UITapGestureRecognizer *)tapGesture;

- (void) setSelectionView : (SelectionView *)selView;

- (void) setProcessPan : (BOOL)p;
- (void) setProcessTap : (BOOL)t;


@end
