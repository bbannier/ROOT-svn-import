#import <UIKit/UIKit.h>

#import "ObjectInspectorComponent.h"

//@class InspectorWithNavigation;
@class ROOTObjectController;
@class AxisLabelsInspector;
@class AxisTicksInspector;
@class AxisTitleInspector;
@class AxisColorInspector;

class TObject;

@interface AxisInspector : UIViewController <ObjectInspectorComponent> {
   AxisTicksInspector *ticksInspector;
   AxisColorInspector *colorInspector;
   
   AxisTitleInspector *titleInspector;
   AxisLabelsInspector *labelsInspector;

   ROOTObjectController *controller;
   TObject *object;
}

+ (CGRect) inspectorFrame;

- (void) setROOTObjectController : (ROOTObjectController *)c;
- (void) setROOTObject : (TObject *)o;
- (NSString *) getComponentName;
- (void) resetInspector;

- (IBAction) showColorInspector;
- (IBAction) showTicksInspector;
- (IBAction) showAxisTitleInspector;
- (IBAction) showAxisLabelsInspector;

@end
