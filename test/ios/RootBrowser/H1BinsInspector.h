#import "ObjectInspectorComponent.h"

@class RangeSlider;

class TH1;

@interface H1BinsInspector : UIViewController <ObjectInspectorComponent> {
@private
   RangeSlider *axisRangeSlider;
      
   IBOutlet UITextField *titleField;
   IBOutlet UISlider *binSlider;
   IBOutlet UILabel *binLabel;
   unsigned maxBins;

   IBOutlet UILabel *minLabel;
   IBOutlet UILabel *maxLabel;
   
   ROOTObjectController *controller;
   TH1 *object;
   TH1 *histClone;
}

@property (nonatomic, retain) UITextField *titleField;
@property (nonatomic, retain) UISlider *binSlider;
@property (nonatomic, retain) UILabel *binLabel;
@property (nonatomic, retain) UILabel *minLabel;
@property (nonatomic, retain) UILabel *maxLabel;

- (void) setROOTObjectController : (ROOTObjectController *)c;
- (void) setROOTObject : (TObject *)o;

- (IBAction) updateNumberOfBins;
//

- (IBAction) textFieldDidEndOnExit : (id) sender;
- (IBAction) textFieldEditingDidEnd : (id) sender;


@end
