//
//  ScrollViewWithPadView.h
//  root_browser
//
//  Created by Timur Pocheptsov on 8/25/11.
//  Copyright 2011 __MyCompanyName__. All rights reserved.
//

#import <UIKit/UIKit.h>

@interface ScrollViewWithPadView : UIScrollView {
   BOOL editMode;
}

@property (nonatomic, assign) BOOL editMode;

@end
