//
//  EditorView.h
//  editors
//
//  Created by Timur Pocheptsov on 8/16/11.
//  Copyright 2011 __MyCompanyName__. All rights reserved.
//

#import <UIKit/UIKit.h>

@class ScrollViewWithPickers;
@class EditorPlateView;

enum {
   evMaxComponents = 5,
   evMaxStates = 1 << evMaxComponents
};

@interface EditorView : UIView {
//   UIScrollView *scrollView;
   ScrollViewWithPickers *scrollView;

   CGFloat plateYs[evMaxStates * evMaxComponents];
   CGFloat viewYs[evMaxStates * evMaxComponents];
   
   UIView *plates[evMaxComponents];
   UIView *views[evMaxComponents];
   UIView *containers[evMaxComponents];   

   unsigned nStates;
   unsigned nEditors;
   unsigned currentState;
   
   BOOL animation;
}

+ (CGFloat) editorAlpha;

+ (CGFloat) editorWidth;
+ (CGFloat) editorHeight;
+ (CGFloat) scrollWidth;
+ (CGFloat) scrollHeight;

- (void) clearEditorView;
- (void) propertyUpdated;
- (void) addSubEditor : (UIView *)element withName : (NSString *)editorName;
- (void) correctFrames;
- (void) plateTapped : (EditorPlateView *) plate;

@end
