//
//  PadGridEditor.h
//  editors
//
//  Created by Timur Pocheptsov on 8/16/11.
//  Copyright 2011 __MyCompanyName__. All rights reserved.
//

#import <UIKit/UIKit.h>

#import "ROOTObjectEditorProtocol.h"

@class ROOTObjectController;

class TVirtualPad;
class TObject;

@interface PadGridEditor : UIViewController <ROOTObjectEditorProtocol> {
   IBOutlet UISwitch *gridX;
   IBOutlet UISwitch *gridY;
   IBOutlet UISwitch *ticksX;
   IBOutlet UISwitch *ticksY;
   
   TVirtualPad *object;
   ROOTObjectController *controller;
}

- (void) setROOTObjectController : (ROOTObjectController *)c;
- (void) setROOTObject : (TObject*)o;

- (IBAction) gridActivated : (UISwitch *) g;
- (IBAction) ticksActivated : (UISwitch *) t;

@end
