#import <UIKit/UIKit.h>

#import "ObjectInspectorComponent.h"

@class ROOTObjectController;
@class AxisColorInspector;

class TAttAxis;
class TObject;

@interface AxisTitleInspector : UIViewController <ObjectInspectorComponent> {
   IBOutlet UITextField *titleField;
   IBOutlet UISwitch *centered;
   IBOutlet UISwitch *rotated;

   ROOTObjectController *controller;
   TObject *object;
   
   AxisColorInspector *colorInspector;
}

+ (CGRect) inspectorFrame;

- (void) setROOTObjectController : (ROOTObjectController *)c;
- (void) setROOTObject : (TObject *)o;

- (IBAction) showTitleFontInspector;
- (IBAction) showTitleColorInspector;

- (IBAction) back;

@end
