#import <UIKit/UIKit.h>

#import "ObjectInspectorComponent.h"

@class ROOTObjectController;

class TAttLine;
class TObject;

//Inspector with a picker for a line style.

@interface LineStyleInspector : UIViewController <UIPickerViewDelegate, UIPickerViewDataSource, ObjectInspectorComponent> {
   IBOutlet UIPickerView *lineStylePicker;
   
   NSMutableArray *lineStyles;

   ROOTObjectController *controller;   
   TAttLine *object;
}

@property (nonatomic, retain) UIPickerView *lineStylePicker;

- (void) setROOTObjectController : (ROOTObjectController *) c;
- (void) setROOTObject : (TObject *) obj;

- (IBAction) back;

@end
