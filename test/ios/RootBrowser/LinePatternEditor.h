//
//  LinePatternEditor.h
//  root_browser
//
//  Created by Timur Pocheptsov on 9/2/11.
//  Copyright 2011 __MyCompanyName__. All rights reserved.
//

#import <UIKit/UIKit.h>

@class ROOTObjectController;

class TAttLine;


@interface LinePatternEditor : UIViewController <UIPickerViewDelegate, UIPickerViewDataSource> {
   IBOutlet UIPickerView *lineStylePicker;
   
   NSMutableArray *lineStyles;

   ROOTObjectController *controller;   
   TAttLine *object;
}

- (void) setController : (ROOTObjectController *) c;
- (void) setObject : (TAttLine *) obj;

- (IBAction) goBack;

@end
