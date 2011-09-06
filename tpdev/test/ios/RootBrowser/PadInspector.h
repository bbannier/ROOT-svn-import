#import <UIKit/UIKit.h>

#import "ObjectInspectorComponent.h"

@class ROOTObjectController;
@class PadTicksGridInspector;
@class PadLogScaleInspector;

class TObject;

@interface PadInspector : UIViewController <ObjectInspectorComponent> {
   PadTicksGridInspector *gridInspector;
   PadLogScaleInspector *logScaleInspector;
   
   ROOTObjectController *controller;
   TObject *object;
}

- (void) setROOTObjectController : (ROOTObjectController *)c;
- (void) setROOTObject : (TObject *)o;
- (NSString *) getComponentName;

- (IBAction) showTicksAndGridInspector;
- (IBAction) showLogScaleInspector;

@end
