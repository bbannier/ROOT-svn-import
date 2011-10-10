#import <UIKit/UIKit.h>


@interface LineStyleCell : UIView {
@private
   unsigned lineStyle;
   
   UIImage *backgroundImage;
}

- (id) initWithFrame : (CGRect)frame lineStyle : (unsigned) style;

@end
