//
//  PadGridEditor.h
//  editors
//
//  Created by Timur Pocheptsov on 8/16/11.
//  Copyright 2011 __MyCompanyName__. All rights reserved.
//

#import <UIKit/UIKit.h>

@class ROOTObjectController;

class TVirtualPad;
class TObject;

@interface PadGridEditor : UIViewController {
   IBOutlet UISwitch *gridX;
   IBOutlet UISwitch *gridY;
   IBOutlet UISwitch *ticksX;
   IBOutlet UISwitch *ticksY;
   
   TVirtualPad *object;
   
   ROOTObjectController *controller;
}

- (void) setController : (ROOTObjectController *)c;
- (void) setObject : (TObject*)o;

- (IBAction) gridActivated : (UISwitch *) g;
- (IBAction) ticksActivated : (UISwitch *) t;

@end
