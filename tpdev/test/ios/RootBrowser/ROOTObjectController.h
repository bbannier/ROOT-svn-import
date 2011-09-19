#import <UIKit/UIKit.h>

@class ScrollViewWithPadView;
@class ObjectInspector;
@class ObjectShortcut;
@class EditorView;
@class PadView;

namespace ROOT_iOS {

class FileContainer;
//Pad to draw object.
class Pad;

}

namespace ROOT_IOSObjectController {

enum Mode {
   ocmNavigation,
   ocmEdit
};

}

//ROOT's object to draw.
class TObject;

@interface ROOTObjectController : UIViewController <UIScrollViewDelegate> {
   ROOT_IOSObjectController::Mode mode;

   EditorView *editorView;
   ObjectInspector *objectInspector;
   
   IBOutlet ScrollViewWithPadView *scrollView;
   UITapGestureRecognizer *doubleTap;
   
   PadView *padView;
   ROOT_iOS::Pad *pad;

   ROOT_iOS::FileContainer *fileContainer;

   TObject *rootObject;
   TObject *selectedObject;
   
   BOOL zoomed;
}

@property (nonatomic, retain) ScrollViewWithPadView *scrollView;

- (void) setObjectWithIndex : (unsigned) index fromContainer : (ROOT_iOS::FileContainer *)fileContainer;
- (void) handleDoubleTapOnPad;
- (void) objectWasSelected : (TObject *)object;
- (void) objectWasModifiedUpdateSelection : (BOOL)needUpdate;
- (void) setupObjectInspector;

@end
