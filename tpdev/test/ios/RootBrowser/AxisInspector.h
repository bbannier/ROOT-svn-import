#import <UIKit/UIKit.h>

#import "ObjectInspectorComponent.h"

//@class InspectorWithNavigation;
@class ROOTObjectController;

class TObject;

@interface AxisInspector : UIViewController <ObjectInspectorComponent> {
   IBOutlet UITabBar *tabBar;

   ROOTObjectController *controller;
   TObject *object;
}

@property (nonatomic, retain) UITabBar *tabBar;

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
