#import <UIKit/UIKit.h>

@interface HorizontalPickerView : UIView <UIScrollViewDelegate> {
   UIScrollView *contentScroll;
   UIImageView *arrowView;
   UIImage *frameImage;
   UIImage *backgroundImage;
}

- (void) addItems : (NSMutableArray *)items;

@end
