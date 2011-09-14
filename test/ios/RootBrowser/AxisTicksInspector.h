#import <UIKit/UIKit.h>

#import "ObjectInspectorComponent.h"

@class ROOTObjectController;

//TAxis, not TAttAxis, since inspector has to work with
//several functions, which are member of TAxis, not TAttAxis.
class TAxis;

@interface AxisTicksInspector : UIViewController <ObjectInspectorComponent> {
   IBOutlet UILabel *tickLengthLabel;
   IBOutlet UIButton *plusLengthBtn;
   IBOutlet UIButton *minusLengthBtn;
   float tickLength;

   IBOutlet UIButton *plusPrim;
   IBOutlet UIButton *minusPrim;
   IBOutlet UILabel *primLabel;
   unsigned primaryTicks;

   IBOutlet UIButton *plusSec;
   IBOutlet UIButton *minusSec;
   IBOutlet UILabel *secLabel;
   unsigned secondaryTicks;

   IBOutlet UIButton *plusTer;
   IBOutlet UIButton *minusTer;
   IBOutlet UILabel *terLabel;
   unsigned tertiaryTicks;

   IBOutlet UISegmentedControl *ticksNegPos;
   
   ROOTObjectController *controller;
   TAxis *object;
}

- (void) setROOTObject : (TObject *)object;
- (void) setROOTObjectController : (ROOTObjectController *)c;

- (IBAction) plusTick : (UIButton *)sender;
- (IBAction) minusTick :(UIButton *)sender;
- (IBAction) ticksNegPos;

- (IBAction) back;

@end
