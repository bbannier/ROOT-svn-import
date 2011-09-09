#import <UIKit/UIKit.h>

#import "ObjectInspectorComponent.h"

@class ROOTObjectController;
@class AxisColorInspector;

class TObject;
class TAxis;

@interface AxisTitleInspector : UIViewController <ObjectInspectorComponent> {
   IBOutlet UITextField *titleField;
   IBOutlet UISwitch *centered;
   IBOutlet UISwitch *rotated;

   ROOTObjectController *controller;
   TAxis *object;

   AxisColorInspector *colorInspector;
}

+ (CGRect) inspectorFrame;

- (void) setROOTObjectController : (ROOTObjectController *)c;
- (void) setROOTObject : (TObject *)o;

- (IBAction) showTitleFontInspector;
- (IBAction) showTitleColorInspector;

- (IBAction) textFieldDidEndOnExit : (id) sender;
- (IBAction) textFieldEditingDidEnd : (id) sender;
- (IBAction) centerTitle;
- (IBAction) rotateTitle;

- (IBAction) back;

@end
