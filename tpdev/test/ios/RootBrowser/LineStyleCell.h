#import <UIKit/UIKit.h>


@interface LineStyleCell : UIView {
   unsigned lineStyle;
   
   UIImage *backgroundImage;
}

- (id) initWithFrame : (CGRect)frame lineStyle : (unsigned) style;

@end
