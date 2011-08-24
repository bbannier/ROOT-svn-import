//
//  ROOTObjectController.h
//  root_browser
//
//  Created by Timur Pocheptsov on 8/19/11.
//  Copyright 2011 __MyCompanyName__. All rights reserved.
//

#import <UIKit/UIKit.h>

@class ObjectShortcut;
@class PadGridEditor;
@class PadLogEditor;
@class EditorView;
@class FillEditor;
@class PadView;

namespace ROOT_iOS {

//Pad to draw object.
class Pad;

}

//ROOT's object to draw.
class TObject;

@interface ROOTObjectController : UIViewController <UIScrollViewDelegate> {
   EditorView *editorView;
   
   PadGridEditor *grid;
   PadLogEditor *log;
   FillEditor *fill;
   
   IBOutlet UIScrollView *scrollView;
   
   PadView *padView;//View for pad.
   ROOT_iOS::Pad *pad;
   TObject *rootObject;
   
   BOOL zoomed;
}

@property (nonatomic, retain) UIScrollView *scrollView;

- (void) setObject : (ObjectShortcut *)object;

@end
