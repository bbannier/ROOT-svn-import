#import <UIKit/UIKit.h>

#import "ObjectInspectorComponent.h"

@class ROOTObjectController;

class TAttFill;
class TObject;

@interface FilledAreaInspector : UIViewController <UIPickerViewDelegate, UIPickerViewDataSource, ObjectInspectorComponent> {
   IBOutlet UIPickerView *fillPicker;
   
   NSMutableArray *colorCells;
   NSMutableArray *patternCells;
   
   TAttFill *filledObject;
   ROOTObjectController *parentController;
}

- (void) setROOTObjectController : (ROOTObjectController *) p;
- (void) setROOTObject : (TObject*) obj;
- (NSString *) getComponentName;


@end
