#import "ObjectInspectorComponent.h"

@class ROOTObjectController;
@class AxisColorInspector;
@class AxisFontInspector;


class TObject;
class TAxis;

@interface AxisLabelsInspector : UIViewController <ObjectInspectorComponent> {

   IBOutlet UIButton *plusSize;
   IBOutlet UIButton *minusSize;
   IBOutlet UILabel *sizeLabel;
   
   IBOutlet UIButton *plusOffset;
   IBOutlet UIButton *minusOffset;
   IBOutlet UILabel *offsetLabel;
   
   IBOutlet UISwitch *noExp;
   
   ROOTObjectController *controller;
   TAxis *object;
   
   AxisColorInspector *colorInspector;
   AxisFontInspector *fontInspector;
}

- (void) setROOTObjectController : (ROOTObjectController *)c;
- (void) setROOTObject : (TObject *)o;

- (IBAction) showLabelFontInspector;
- (IBAction) showLabelColorInspector;

- (IBAction) plusBtn : (UIButton *)sender;
- (IBAction) minusBtn : (UIButton *)sender;
- (IBAction) noExp;
- (IBAction) back;

@end
