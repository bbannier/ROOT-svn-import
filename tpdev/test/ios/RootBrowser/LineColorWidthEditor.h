//
//  LineColorWidthEditor.h
//  root_browser
//
//  Created by Timur Pocheptsov on 9/2/11.
//  Copyright 2011 __MyCompanyName__. All rights reserved.
//

#import <UIKit/UIKit.h>

#import "ROOTObjectEditorProtocol.h"

@class ROOTObjectController;

class TAttLine;
class TObject;

@interface LineColorWidthEditor : UIViewController <UIPickerViewDelegate, UIPickerViewDataSource, ROOTObjectEditorProtocol> {
   IBOutlet UIPickerView *linePicker;

   NSMutableArray *lineColors;
   NSMutableArray *lineWidths;

   ROOTObjectController *controller;
   TAttLine *object;
}

- (IBAction) goBack;
- (void) setROOTObjectController : (ROOTObjectController *) c;
- (void) setROOTObject : (TObject *) obj;


@end
