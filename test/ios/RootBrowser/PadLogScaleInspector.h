#import <UIKit/UIKit.h>

#import "ObjectInspectorComponent.h"

@class ROOTObjectController;
class TVirtualPad;
class TObject;

@interface PadLogScaleInspector : UIViewController <ObjectInspectorComponent> {
   IBOutlet UISwitch *logX;
   IBOutlet UISwitch *logY;
   IBOutlet UISwitch *logZ;
   
   ROOTObjectController *controller;
   TVirtualPad *object;
}

- (void) setROOTObjectController : (ROOTObjectController *)c;
- (void) setROOTObject : (TObject *)o;

- (IBAction) logActivated : (UISwitch *) log;
- (IBAction) back;

@end
