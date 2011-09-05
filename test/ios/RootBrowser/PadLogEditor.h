//
//  PadLogEditor.h
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

@interface PadLogEditor : UIViewController <ROOTObjectEditorProtocol> {
   IBOutlet UISwitch *logX;
   IBOutlet UISwitch *logY;
   IBOutlet UISwitch *logZ;
   
   ROOTObjectController *controller;
   TVirtualPad *object;
}

- (void) setROOTObjectController : (ROOTObjectController *)c;
- (void) setROOTObject : (TObject *)o;

- (IBAction) logActivated : (UISwitch *) log;

@end
