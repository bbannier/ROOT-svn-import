//
//  AxisInspector.h
//  root_browser
//
//  Created by Timur Pocheptsov on 9/7/11.
//  Copyright 2011 __MyCompanyName__. All rights reserved.
//

#import <UIKit/UIKit.h>

#import "ObjectInspectorComponent.h"

@class ROOTObjectController;
@class AxisTicksInspector;
@class AxisColorInspector;

class TObject;

@interface AxisInspector : UIViewController <ObjectInspectorComponent> {
   AxisTicksInspector *ticksInspector;
   AxisColorInspector *colorInspector;
   
   ROOTObjectController *controller;
   TObject *object;
}

+ (CGRect) inspectorFrame;

- (void) setROOTObjectController : (ROOTObjectController *)c;
- (void) setROOTObject : (TObject *)o;
- (NSString *) getComponentName;

- (IBAction) showColorInspector;
- (IBAction) showTicksInspector;

@end
