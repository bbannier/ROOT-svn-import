//
//  FillEditor.h
//  editors
//
//  Created by Timur Pocheptsov on 8/16/11.
//  Copyright 2011 __MyCompanyName__. All rights reserved.
//

#import <UIKit/UIKit.h>

@class ROOTObjectController;

class TAttFill;
class TObject;

@interface FillEditor : UIViewController {
   IBOutlet UIPickerView *colorPicker;
   IBOutlet UIPickerView *patternPicker;
   
   NSMutableArray *colorCells;
   NSMutableArray *patternCells;
   
   TAttFill *filledObject;
   
   ROOTObjectController *parentController;
}

- (void) setController : (ROOTObjectController *) p;
- (void) setObject : (TObject*) obj;


@end
