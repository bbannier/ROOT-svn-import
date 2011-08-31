//
//  LineStyleCell.h
//  root_browser
//
//  Created by Timur Pocheptsov on 8/31/11.
//  Copyright 2011 __MyCompanyName__. All rights reserved.
//

#import <UIKit/UIKit.h>


@interface LineStyleCell : UIView {
   unsigned lineStyle;
   
   UIImage *backgroundImage;
}

- (id) initWithFrame : (CGRect)frame lineStyle : (unsigned) style;

@end
