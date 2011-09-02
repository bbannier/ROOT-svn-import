//
//  LineColorWidthEditor.h
//  root_browser
//
//  Created by Timur Pocheptsov on 9/2/11.
//  Copyright 2011 __MyCompanyName__. All rights reserved.
//

#import <UIKit/UIKit.h>

@class ROOTObjectController;

class TAttLine;

@interface LineColorWidthEditor : UIViewController <UIPickerViewDelegate, UIPickerViewDataSource> {
   IBOutlet UIPickerView *lineColorWidthPicker;

   NSMutableArray *lineColors;
   NSMutableArray *lineWidths;

   ROOTObjectController *controller;
   TAttLine *object;
}

- (IBAction) goBack;
- (void) setController : (ROOTObjectController *) c;
- (void) setObject : (TAttLine *) obj;


@end
