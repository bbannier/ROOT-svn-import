#import <UIKit/UIKit.h>

@protocol ThumbnailViewDatasource;

namespace ROOT {
namespace iOS {
namespace Browser {

class FileContainer;

}
}
}

@interface ThumbnailView : UIView

@property (nonatomic, weak) id<ThumbnailViewDatasource> dataSource;

- (void) loadThumbnails;
- (void) configureView;

@end

