#import <UIKit/UIKit.h>

@interface LineWidthCell : UIView {
   CGFloat lineWidth;
   
   UIImage *backgroundImage;
}

- (id) initWithFrame : (CGRect) frame width : (CGFloat) lineWidth;

@end
