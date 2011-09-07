#import <UIKit/UIKit.h>

#import "ObjectInspectorComponent.h"

@class ROOTObjectController;

class TAttAxis;

@interface AxisColorInspector : UIViewController <UIPickerViewDelegate, UIPickerViewDataSource, ObjectInspectorComponent> {
   IBOutlet UIPickerView *colorPicker;
   
   NSMutableArray *colors;
   
   ROOTObjectController *controller;
   TAttAxis *object;
}

- (void) setROOTObjectController : (ROOTObjectController *)contoller;
- (void) setROOTObject : (TObject *)object;

- (IBAction) back;

@end
