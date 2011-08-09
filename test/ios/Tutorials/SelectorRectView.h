//
//  SelectorRectView.h
//  Tutorials
//
//  Created by Timur Pocheptsov on 8/9/11.
//  Copyright 2011 __MyCompanyName__. All rights reserved.
//

#import <UIKit/UIKit.h>

//
// Transparent view with a selector's "glass" rectangle
//

@interface SelectorRectView : UIView {
   unsigned col;
   unsigned row;

   UIImage *glassImage;
}

@property (nonatomic, retain) UIImage *glassImage;

- (void) setCol : (unsigned) col andRow : (unsigned) row;

@end
