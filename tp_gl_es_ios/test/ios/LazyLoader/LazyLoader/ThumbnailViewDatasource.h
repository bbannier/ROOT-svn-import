#import <Foundation/Foundation.h>

@class ThumbnailView;

@protocol ThumbnailViewDatasource <NSObject>
- (NSInteger) numberOfItemsInView : (ThumbnailView *)view;
- (UIView *) viewForItemInView : (ThumbnailView *)view atIndex : (NSInteger)index;
- (void) loadThumbnailForView : (UIView *)view;
@end

