#import <UIKit/UIKit.h>

#import "ObjectInspectorComponent.h"

@class ROOTObjectController;

class TAttLine;
class TObject;

//Inspector (component) with picker for line color and width selection.
//Picker has 2 components. Component 0 - color picker,
//it has 16 colors, as in ROOT's standard color pick in TCanvas editor.
//Component 1 - line width picker.

@interface LineColorWidthInspector : UIViewController <UIPickerViewDelegate, UIPickerViewDataSource, ObjectInspectorComponent> {
   IBOutlet UIPickerView *linePicker;

   NSMutableArray *lineColors;
   NSMutableArray *lineWidths;

   ROOTObjectController *controller;
   TAttLine *object;
}

@property (nonatomic, retain) UIPickerView *linePicker;

- (IBAction) back;

- (void) setROOTObjectController : (ROOTObjectController *) c;
- (void) setROOTObject : (TObject *) obj;

@end
