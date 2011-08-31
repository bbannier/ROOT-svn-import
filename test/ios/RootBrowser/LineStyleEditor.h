//
//  LineStyleEditor.h
//  root_browser
//
//  Created by Timur Pocheptsov on 8/31/11.
//  Copyright 2011 __MyCompanyName__. All rights reserved.
//

#import <UIKit/UIKit.h>


@interface LineStyleEditor : UIViewController <UIPickerViewDelegate, UIPickerViewDataSource> {
   IBOutlet UIPickerView *lineColorWidthPicker;
   IBOutlet UIPickerView *lineStylePicker;
   
   NSMutableArray *lineColors;
   NSMutableArray *lineWidths;
   NSMutableArray *lineStyles;
}

@end
