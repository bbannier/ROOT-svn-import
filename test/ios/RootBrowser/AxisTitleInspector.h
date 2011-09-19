#import <UIKit/UIKit.h>

#import "ObjectInspectorComponent.h"

@class ROOTObjectController;

class TObject;
class TAxis;

@interface AxisTitleInspector : UIViewController <ObjectInspectorComponent> {
   IBOutlet UITextField *titleField;
   IBOutlet UISwitch *centered;
   IBOutlet UISwitch *rotated;

   ROOTObjectController *controller;
   TAxis *object;

   IBOutlet UILabel *offsetLabel;
   IBOutlet UIButton *plusOffsetBtn;
   IBOutlet UIButton *minusOffsetBtn;
   float offset;
}

@property (nonatomic, retain) UITextField *titleField;
@property (nonatomic, retain) UISwitch *centered;
@property (nonatomic, retain) UISwitch *rotated;
@property (nonatomic, retain) UILabel *offsetLabel;
@property (nonatomic, retain) UIButton *plusOffsetBtn;
@property (nonatomic, retain) UIButton *minusOffsetBtn;

+ (CGRect) inspectorFrame;

- (void) setROOTObjectController : (ROOTObjectController *)c;
- (void) setROOTObject : (TObject *)o;

- (IBAction) showTitleFontInspector;
- (IBAction) showTitleColorInspector;

- (IBAction) textFieldDidEndOnExit : (id) sender;
- (IBAction) textFieldEditingDidEnd : (id) sender;
- (IBAction) centerTitle;
- (IBAction) rotateTitle;
- (IBAction) plusOffset;
- (IBAction) minusOffset;

- (IBAction) back;

@end
