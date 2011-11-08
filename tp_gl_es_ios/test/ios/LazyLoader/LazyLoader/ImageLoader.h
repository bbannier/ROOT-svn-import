#import <UIKit/UIKit.h>

namespace ROOT {
namespace iOS {
namespace Browser {

class FileContainer;

}
}
}

@interface ImageLoader : NSOperation

- (id) initWithContainer : (ROOT::iOS::Browser::FileContainer *)container andViews : (NSMutableSet *)views;

@end
