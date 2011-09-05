#import <UIKit/UIKit.h>

#import "ROOTObjectEditorProtocol.h"

@class ROOTObjectController;

class TAttFill;
class TObject;

//
//FillEditor is a small panel to edit TAttLine with a picker.
//Picker consists of two components - one to select a color,
//the second to select a fill pattern. Since ROOT's code
//to define palettes, colors, styles etc. is a bit messy and
//is dependant on the mode (batch or not), 
//color picker contains only 16 colors from standard "color pick" 
//widget or how they call it (widget from TCanvas' editor).
//


@interface FillEditor : UIViewController <ROOTObjectEditorProtocol> {
   IBOutlet UIPickerView *fillPicker;
   
   NSMutableArray *colorCells;
   NSMutableArray *patternCells;
   
   TAttFill *filledObject;
   ROOTObjectController *parentController;
}

- (void) setROOTObjectController : (ROOTObjectController *) p;
- (void) setROOTObject : (TObject*) obj;


@end
