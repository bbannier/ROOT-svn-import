//
//  PatternPickerView.h
//  Tutorials
//
//  Created by Timur Pocheptsov on 8/9/11.
//  Copyright 2011 __MyCompanyName__. All rights reserved.
//

#import <UIKit/UIKit.h>


@interface PatternPickerView : UIView {
   UIImage *backgroundImage;
}

@property (nonatomic, retain) UIImage *backgroundImage;

- (void) setupPatternPicker;

@end
