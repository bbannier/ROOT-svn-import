//
//  LineAttribEditor.h
//  root_browser
//
//  Created by Timur Pocheptsov on 9/2/11.
//  Copyright 2011 __MyCompanyName__. All rights reserved.
//

#import <UIKit/UIKit.h>


@class ROOTObjectController;

class TAttLine;

@interface LineAttribEditor : UITableViewController {
   ROOTObjectController *controller;
   TAttLine *object;
}

- (id)initWithStyle : (UITableViewStyle)style controller : (ROOTObjectController *)c;
- (void) setObject : (TObject *) obj;

@end
