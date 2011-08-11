//
//  PadOptionsController.h
//  Tutorials
//
//  Created by Timur Pocheptsov on 8/11/11.
//  Copyright 2011 __MyCompanyName__. All rights reserved.
//

#import <UIKit/UIKit.h>


@interface PadOptionsController : UIViewController {
   IBOutlet UISwitch *tickX_;
   IBOutlet UISwitch *tickY_;

   IBOutlet UISwitch *gridX_;
   IBOutlet UISwitch *gridY_;

   IBOutlet UISwitch *logX_;
   IBOutlet UISwitch *logY_;
   IBOutlet UISwitch *logZ_;
   
   IBOutlet UIPickerView *colorPicker_;
   IBOutlet UIPickerView *patternPicker_;
   
   NSMutableArray *colors_;
}

@property (nonatomic, retain) UISwitch *tickX;
@property (nonatomic, retain) UISwitch *tickY;
@property (nonatomic, retain) UISwitch *gridX;
@property (nonatomic, retain) UISwitch *gridY;
@property (nonatomic, retain) UISwitch *logX;
@property (nonatomic, retain) UISwitch *logY;
@property (nonatomic, retain) UISwitch *logZ;
@property (nonatomic, retain) UIPickerView *colorPicker;
@property (nonatomic, retain) UIPickerView *patternPicker;
@property (nonatomic, retain) NSMutableArray *colors;

@end
