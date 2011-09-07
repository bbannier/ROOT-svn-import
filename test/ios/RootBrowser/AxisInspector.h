//
//  AxisInspector.h
//  root_browser
//
//  Created by Timur Pocheptsov on 9/7/11.
//  Copyright 2011 __MyCompanyName__. All rights reserved.
//

#import <UIKit/UIKit.h>

#import "ObjectInspectorComponent.h"

//@class InspectorWithNavigation;
@class ROOTObjectController;
@class AxisTicksInspector;
@class AxisTitleInspector;
@class AxisColorInspector;

class TObject;

@interface AxisInspector : UIViewController <ObjectInspectorComponent> {
   AxisTicksInspector *ticksInspector;
   AxisColorInspector *colorInspector;
   
   AxisTitleInspector *titleInspector;
//   InspectorWithNavigation *labelsInspector;

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
