#import <UIKit/UIKit.h>

#import "HorizontalPickerDelegate.h"

@interface HorizontalPickerView : UIView <UIScrollViewDelegate> {
   UIScrollView *contentScroll;
   UIImageView *arrowView;
   UIImage *frameImage;
   UIImage *backgroundImage;
   
   unsigned selectedItem;
}

@property (nonatomic, weak) id<HorizontalPickerDelegate> pickerDelegate;

- (void) addItems : (NSMutableArray *)items;
- (void) setSelectedItem : (unsigned) item;

@end
