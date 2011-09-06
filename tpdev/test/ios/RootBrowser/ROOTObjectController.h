//
//  ROOTObjectController.h
//  root_browser
//
//  Created by Timur Pocheptsov on 8/19/11.
//  Copyright 2011 __MyCompanyName__. All rights reserved.
//

#import <UIKit/UIKit.h>

@class InspectorWithNavigation;
@class FilledAreaInspector;
@class ScrollViewWithPadView;
@class ObjectShortcut;
@class EditorView;
@class PadView;

namespace ROOT_iOS {

//Pad to draw object.
class Pad;

}

//ROOT's object to draw.
class TObject;

@interface ROOTObjectController : UIViewController <UIScrollViewDelegate> {
   EditorView *editorView;
   
   InspectorWithNavigation *lineInspector;
   InspectorWithNavigation *padInspector;
   FilledAreaInspector *fillInspector;
   
   IBOutlet ScrollViewWithPadView *scrollView;
   
   PadView *padView;//View for pad.
   ROOT_iOS::Pad *pad;
   TObject *rootObject;
   TObject *selectedObject;
   
   unsigned currentEditors;
   
   BOOL zoomed;
}

@property (nonatomic, retain) UIScrollView *scrollView;

- (void) setObjectFromShortcut : (ObjectShortcut *)object;
- (void) handleDoubleTapOnPad;
- (void) objectWasSelected : (TObject *)object;
- (void) objectWasModifiedByEditor;
- (void) addObjectEditors;
- (void) setEditorValues;

@end
