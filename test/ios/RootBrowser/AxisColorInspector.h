#import <UIKit/UIKit.h>

#import "ObjectInspectorComponent.h"

@class ROOTObjectController;

class TAttAxis;

namespace ROOT_IOSObjectInspector {

enum AxisColorInspectorMode {
   acimAxisColor,
   acimTitleColor,
   acimLabelColor
};

}
@interface AxisColorInspector : UIViewController <UIPickerViewDelegate, UIPickerViewDataSource, ObjectInspectorComponent> {
   ROOT_IOSObjectInspector::AxisColorInspectorMode mode;

   IBOutlet UIPickerView *colorPicker;
   IBOutlet UILabel *titleLabel;
   
   NSMutableArray *colors;
   
   ROOTObjectController *controller;
   TAttAxis *object;
}

- (id) initWithNibName : (NSString *)nibNameOrNil bundle : (NSBundle *)nibBundleOrNil mode : (ROOT_IOSObjectInspector::AxisColorInspectorMode)mode;

- (void) setROOTObjectController : (ROOTObjectController *)contoller;
- (void) setROOTObject : (TObject *)object;

- (IBAction) back;

@end
