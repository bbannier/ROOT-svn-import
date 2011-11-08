#import <UIKit/UIKit.h>

#import "ThumbnailView.h"

@class ThumbnailView;

@interface ViewController : UIViewController<ThumbnailViewDelegate>

//Adapting the ThumbnailViewDelegate's protocol:
- (unsigned) numberOfThumbnailsInView : (ThumbnailView *)view;
- (UIView *) thumbnailAtIndex : (unsigned)index;
- (void) cacheDataForThumbnail : (UIView *)view;
//- (void) loadDataForVisibleRange;

- (void) loadThumbnailForView : (UIView *)view;
//

@end
