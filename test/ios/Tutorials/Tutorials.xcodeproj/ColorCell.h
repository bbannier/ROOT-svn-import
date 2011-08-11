//
//  ColorCell.h
//  Tutorials
//
//  Created by Timur Pocheptsov on 8/11/11.
//  Copyright 2011 __MyCompanyName__. All rights reserved.
//

#import <UIKit/UIKit.h>


@interface ColorCell : UIView {
   unsigned colorIndex;
   float rgb[3];
}

- (void) setColorID : (unsigned) colorID andRGB : (const double *) rgb;

@end
