//
//  LineAttribEditor.h
//  root_browser
//
//  Created by Timur Pocheptsov on 9/2/11.
//  Copyright 2011 __MyCompanyName__. All rights reserved.
//

#import <UIKit/UIKit.h>

#import "ROOTObjectEditorProtocol.h"

@class ROOTObjectController;

class TObject;

@interface LineAttribEditor : UITableViewController <ROOTObjectEditorProtocol> {
   ROOTObjectController *controller;
   TObject *object;
}

- (id)initWithStyle : (UITableViewStyle)style controller : (ROOTObjectController *)c;

- (void) setROOTObjectController : (ROOTObjectController *) c;
- (void) setROOTObject : (TObject *) obj;

@end
