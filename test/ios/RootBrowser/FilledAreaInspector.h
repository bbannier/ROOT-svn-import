#import <UIKit/UIKit.h>

#import "ObjectInspectorComponent.h"

@class ROOTObjectController;

class TAttFill;
class TObject;

@interface FilledAreaInspector : UIViewController <UIPickerViewDelegate, UIPickerViewDataSource, ObjectInspectorComponent> {
@private
   IBOutlet UIPickerView *fillPicker;
   
   NSMutableArray *colorCells;
   NSMutableArray *patternCells;
   
   TAttFill *filledObject;
   ROOTObjectController *parentController;
}

@property (nonatomic, retain) UIPickerView *fillPicker;

- (void) setROOTObjectController : (ROOTObjectController *) p;
- (void) setROOTObject : (TObject*) obj;
- (NSString *) getComponentName;


@end
