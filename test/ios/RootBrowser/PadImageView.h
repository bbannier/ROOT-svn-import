#import <UIKit/UIKit.h>

@interface PadImageView : UIView {
@private
   UIImage *padImage;
   BOOL zoomed;
}

@property (assign) BOOL zoomed;

- (void) setPadImage : (UIImage *)image;
- (UIImage *) getPadImage;

@end
