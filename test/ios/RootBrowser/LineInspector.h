#import <UIKit/UIKit.h>

#import "ObjectInspectorComponent.h"


//Forward declarations for ROOTObjectController and TObject are here, 
//for the case ObjectInspectorComponent protocol
//changes and forward declarations there will be removed.

@class ROOTObjectController;
@class LineColorWidthInspector;
@class LineStyleInspector;

class TObject;

//Line inspector is a composition of two sub-inspectors: line color and width inspector + 
//line style inspector.

@interface LineInspector : UIViewController <ObjectInspectorComponent> {
   LineColorWidthInspector *colorWidthInspector;
   LineStyleInspector *styleInspector;
   
   ROOTObjectController *controller;
   TObject *object;
}

- (void) setROOTObjectController : (ROOTObjectController *)c;
- (void) setROOTObject : (TObject *)o;
- (NSString *) getComponentName;

- (IBAction) showColorWidthComponent;
- (IBAction) showStyleComponent;

@end
