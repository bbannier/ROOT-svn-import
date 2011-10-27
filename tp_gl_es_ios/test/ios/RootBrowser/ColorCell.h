#import <UIKit/UIKit.h>

@interface ColorCell : UIView {
@private
   float rgb[3];
}

- (void) setRGB : (const double *) rgb;
+ (CGFloat) cellAlpha;

@end
