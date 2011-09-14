#import <UIKit/UIKit.h>

#import "ObjectInspectorComponent.h"

@class ROOTObjectController;

class TVirtualPad;
class TObject;

@interface PadTicksGridInspector : UIViewController <ObjectInspectorComponent> {
   IBOutlet UISwitch *gridX;
   IBOutlet UISwitch *gridY;
   IBOutlet UISwitch *ticksX;
   IBOutlet UISwitch *ticksY;
   
   ROOTObjectController *controller;
   TVirtualPad *object;
}

- (void) setROOTObjectController : (ROOTObjectController *) c;
- (void) setROOTObject : (TObject *) obj;

- (IBAction) gridActivated : (UISwitch *) g;
- (IBAction) ticksActivated : (UISwitch *) t;
- (IBAction) back;


@end
