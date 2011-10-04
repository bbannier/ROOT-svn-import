#import <UIKit/UIKit.h>

#import "ObjectInspectorComponent.h"

class TAttMarker;

@interface MarkerInspector : UIViewController <UIPickerViewDelegate, UIPickerViewDataSource, ObjectInspectorComponent> {
   IBOutlet UIPickerView *markerPicker;
   
   NSMutableArray *cells;
   
   IBOutlet UIButton *plusBtn;
   IBOutlet UIButton *minusBtn;
   IBOutlet UILabel *sizeLabel;
   
   ROOTObjectController *controller;
   TAttMarker *object;
}

@property (nonatomic, retain) UIPickerView *markerPicker;
@property (nonatomic, retain) UIButton *plusBtn;
@property (nonatomic, retain) UIButton *minusBtn;
@property (nonatomic, retain) UILabel *sizeLabel;

- (void) setROOTObjectController : (ROOTObjectController *)c;
- (void) setROOTObject : (TObject *)o;
- (NSString *) getComponentName;

- (IBAction) plusPressed;
- (IBAction) minusPressed;

@end
