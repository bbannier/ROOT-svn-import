//
//  PadGridEditor.h
//  editors
//
//  Created by Timur Pocheptsov on 8/16/11.
//  Copyright 2011 __MyCompanyName__. All rights reserved.
//

#import <UIKit/UIKit.h>


@interface PadGridEditor : UIViewController {
   IBOutlet UISwitch *gridX;
   IBOutlet UISwitch *gridY;
   IBOutlet UISwitch *ticksX;
   IBOutlet UISwitch *ticksY;
}

- (IBAction) gridActivated : (UISwitch *) g;
- (IBAction) ticksActivated : (UISwitch *) t;

@end
