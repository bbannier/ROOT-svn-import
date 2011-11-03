#import <UIKit/UIKit.h>

namespace ROOT {
namespace iOS {
namespace Browser {

enum class ObjectType;

}
}
}

@interface ObjectShortcut : UIView

@property (nonatomic, assign) unsigned viewTag;
@property (nonatomic, retain) NSString *objectName;

+ (CGFloat) imageWidth;
+ (CGFloat) imageHeight;
+ (CGFloat) textHeight;

- (void) setObjectType : (ROOT::iOS::Browser::ObjectType)type;

@end
