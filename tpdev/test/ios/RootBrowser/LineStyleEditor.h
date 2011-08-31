//
//  LineStyleEditor.h
//  root_browser
//
//  Created by Timur Pocheptsov on 8/31/11.
//  Copyright 2011 __MyCompanyName__. All rights reserved.
//

#import <UIKit/UIKit.h>

@class ROOTObjectController;

class TAttLine;
class TObject;

@interface LineStyleEditor : UIViewController <UIPickerViewDelegate, UIPickerViewDataSource> {
   IBOutlet UIPickerView *lineColorWidthPicker;
   IBOutlet UIPickerView *lineStylePicker;
   
   NSMutableArray *lineColors;
   NSMutableArray *lineWidths;
   NSMutableArray *lineStyles;

   ROOTObjectController *controller;
   
   TAttLine *object;
}

- (void) setController : (ROOTObjectController *) c;
- (void) setObject : (TObject *) obj;

@end
