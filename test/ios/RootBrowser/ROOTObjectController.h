#import <MessageUI/MessageUI.h>
#import <UIKit/UIKit.h>

#import "IOSFileContainer.h"

@class ScrollViewWithPadView;
@class PadImageScrollView;
@class ObjectInspector;
@class ObjectShortcut;
@class EditorView;
@class PadView;

namespace ROOT_iOS {

//Pad to draw object.
class Pad;

}


////////////

namespace ROOT_IOSObjectController {

enum Mode {
   ocmNavigation,
   ocmEdit
};

}

//ROOT's object to draw.
class TObject;

@interface ROOTObjectController : UIViewController <UIScrollViewDelegate, MFMailComposeViewControllerDelegate> {
@private
   ROOT_IOSObjectController::Mode mode;

   EditorView *editorView;
   ObjectInspector *objectInspector;
   
   IBOutlet ScrollViewWithPadView *scrollView;
   IBOutlet UIScrollView *navigationScrollView;
   
   PadView *editablePadView;

   ROOT_iOS::FileContainer *fileContainer;

   TObject *selectedObject;
   
   BOOL zoomed;
   
   PadImageScrollView *navScrolls[3];

   unsigned currentObject;
   unsigned nextObject;
   unsigned previousObject;
   
   UIBarButtonItem *editBtn;
   
}

@property (nonatomic, retain) ScrollViewWithPadView *scrollView;
@property (nonatomic, retain) UIScrollView *navigationScrollView;


- (void) setNavigationForObjectWithIndex : (unsigned) index fromContainer : (ROOT_iOS::FileContainer *)fileContainer;
- (void) handleDoubleTapOnPad : (CGPoint)tapPt;
- (void) objectWasSelected : (TObject *)object;
- (void) objectWasModifiedUpdateSelection : (BOOL)needUpdate;
- (void) setupObjectInspector;

- (ROOT_iOS::EHistogramErrorOption) getErrorOption;
- (void) setErrorOption : (ROOT_iOS::EHistogramErrorOption) errorOption;

- (BOOL) markerIsOn;
- (void) setMarker : (BOOL)on;

@end
