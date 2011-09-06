//
//  ObjectInspectorComponent.h
//  root_browser
//
//  Created by Timur Pocheptsov on 9/6/11.
//  Copyright 2011 __MyCompanyName__. All rights reserved.
//

#import <Foundation/Foundation.h>

@class ROOTObjectController;

class TObject;

@protocol ObjectInspectorComponent <NSObject>

- (void) setROOTObjectController : (ROOTObjectController *)c;
- (void) setROOTObject : (TObject *)o;

@optional

- (NSString*) getComponentName;
- (void) resetInspector;

@end
