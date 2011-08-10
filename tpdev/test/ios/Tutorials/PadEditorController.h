//
//  PadEditorController.h
//  Tutorials
//
//  Created by Timur Pocheptsov on 8/10/11.
//  Copyright 2011 __MyCompanyName__. All rights reserved.
//

#import <Foundation/Foundation.h>


@interface PadEditorController : UIViewController {
   IBOutlet UIView *colorSelector;
   IBOutlet UIView *patternSelector;
   
   IBOutlet UISwitch *tickX;
   IBOutlet UISwitch *tickY;
   
   IBOutlet UISwitch *gridX;
   IBOutlet UISwitch *gridY;
   
   IBOutlet UISwitch *logX;
   IBOutlet UISwitch *logY;
   IBOutlet UISwitch *logZ;
}

- (void) report;

- (IBAction) switchActivated : (id) control;

@end
