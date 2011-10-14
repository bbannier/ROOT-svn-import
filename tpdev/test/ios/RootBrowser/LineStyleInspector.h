#import <UIKit/UIKit.h>

#import "ObjectInspectorComponent.h"

@class ROOTObjectController;
@class HorizontalPickerView;
@class LineWidthPicker;

class TAttLine;
class TObject;

//Inspector with a picker for a line style.

@interface LineStyleInspector : UIViewController <ObjectInspectorComponent> {
@private
   NSMutableArray *lineStyles;
   NSMutableArray *lineColors;

   HorizontalPickerView *lineStylePicker;
   HorizontalPickerView *lineColorPicker;

   ROOTObjectController *controller;
   TAttLine *object;
   
   IBOutlet LineWidthPicker *lineWidthView;
}

@property (nonatomic, retain) LineWidthPicker *lineWidthView;

- (void) setROOTObjectController : (ROOTObjectController *) c;
- (void) setROOTObject : (TObject *) obj;

- (IBAction) decLineWidth;
- (IBAction) incLineWidth;

@end
