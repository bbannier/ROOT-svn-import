#import <Foundation/Foundation.h>

@class ObjectShortcut;

@protocol ThumbnailViewDatasource <NSObject>
- (ObjectShortcut *) thumbnailAtIndex : (unsigned)index;
- (unsigned) numberOfThumbnails;
@end
