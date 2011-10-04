#import <MessageUI/MessageUI.h>
#import <UIKit/UIKit.h>

@class ScrollViewWithPadView;
@class PadImageScrollView;
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

@interface ROOTObjectController : UIViewController <UIScrollViewDelegate, MFMailComposeViewControllerDelegate> {
@private
   ROOT_IOSObjectController::Mode mode;

   EditorView *editorView;
   ObjectInspector *objectInspector;
   
   IBOutlet ScrollViewWithPadView *scrollView;
   IBOutlet UIScrollView *navigationScrollView;
   
   PadView *padView;
   ROOT_iOS::Pad *pad;

   ROOT_iOS::FileContainer *fileContainer;

   TObject *selectedObject;
   
   BOOL zoomed;
   
   PadImageScrollView *navScrolls[3];
   ROOT_iOS::Pad *navPad;
   unsigned currentObject;
   unsigned nextObject;
   unsigned previousObject;
   
   UIBarButtonItem *editBtn;
   
   //Option like lego, surf, etc.
   NSString *basicDrawOption;
   //E-E5 options.
   NSString *errorsDrawOption;
   //P option.
   NSString *markerDrawOption;
   //May be option like CYL, POL, SPH, etc.
}

@property (nonatomic, retain) ScrollViewWithPadView *scrollView;
@property (nonatomic, retain) UIScrollView *navigationScrollView;
@property (nonatomic, retain) NSString *basicDrawOption;
@property (nonatomic, retain) NSString *errorsDrawOption;
@property (nonatomic, retain) NSString *markerDrawOption;


- (void) setNavigationForObjectWithIndex : (unsigned) index fromContainer : (ROOT_iOS::FileContainer *)fileContainer;
- (void) handleDoubleTapOnPad : (CGPoint)tapPt;
- (void) objectWasSelected : (TObject *)object;
- (void) objectWasModifiedUpdateSelection : (BOOL)needUpdate;
- (void) setupObjectInspector;

//Draw option for the controlled object.
- (NSString *) getDrawOption;

@end
