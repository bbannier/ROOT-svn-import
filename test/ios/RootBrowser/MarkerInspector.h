#import <UIKit/UIKit.h>

#import "ObjectInspectorComponent.h"

class TAttMarker;

@interface MarkerInspector : UIViewController <UIPickerViewDelegate, UIPickerViewDataSource, ObjectInspectorComponent> {
   IBOutlet UIPickerView *markerPicker;
   
   ROOTObjectController *controller;
   TAttMarker *object;
}

@property (nonatomic, retain) UIPickerView *markerPicker;

@end
