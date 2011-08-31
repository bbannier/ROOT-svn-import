//
//  PadLogEditor.h
//  editors
//
//  Created by Timur Pocheptsov on 8/16/11.
//  Copyright 2011 __MyCompanyName__. All rights reserved.
//

#import <UIKit/UIKit.h>

@class ROOTObjectController;

class TVirtualPad;
class TObject;

@interface PadLogEditor : UIViewController {
   IBOutlet UISwitch *logX;
   IBOutlet UISwitch *logY;
   IBOutlet UISwitch *logZ;
   
   ROOTObjectController *controller;
   TVirtualPad *object;
}

- (void) setController : (ROOTObjectController *)c;
- (void) setObject : (TObject *)o;

- (IBAction) logActivated : (UISwitch *) log;

@end
