//
//  InspectorWithNavigation.h
//  root_browser
//
//  Created by Timur Pocheptsov on 9/6/11.
//  Copyright 2011 __MyCompanyName__. All rights reserved.
//

#import <UIKit/UIKit.h>

#import "ObjectInspectorComponent.h"

@class ROOTObjectController;

class TObject;

@interface InspectorWithNavigation : UINavigationController <ObjectInspectorComponent>

- (void) setROOTObjectController : (ROOTObjectController *)c;
- (void) setROOTObject : (TObject *)obj;
- (NSString *) getComponentName;
- (void) resetInspector;

@end
