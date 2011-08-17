//
//  FillEditor.h
//  editors
//
//  Created by Timur Pocheptsov on 8/16/11.
//  Copyright 2011 __MyCompanyName__. All rights reserved.
//

#import <UIKit/UIKit.h>

@class NSMutableArray;

class TAttFill;

@interface FillEditor : UIViewController {
   IBOutlet UIPickerView *colorPicker;
   IBOutlet UIPickerView *patternPicker;
   
   NSMutableArray *colorCells;
   NSMutableArray *patternCells;
   
   TAttFill *filledObject;
   
   id parentController;
}

- (void) setParentController : (id) p;
- (void) setObject : (TAttFill*) obj;


@end
