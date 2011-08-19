//
//  PadLogEditor.h
//  editors
//
//  Created by Timur Pocheptsov on 8/16/11.
//  Copyright 2011 __MyCompanyName__. All rights reserved.
//

#import <UIKit/UIKit.h>


@interface PadLogEditor : UIViewController {
   IBOutlet UISwitch *logX;
   IBOutlet UISwitch *logY;
   IBOutlet UISwitch *logZ;
}

- (IBAction) logActivated : (UISwitch *) log;

@end
