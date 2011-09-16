#import <UIKit/UIKit.h>

namespace ROOT_iOS {

class Pad;

}

@interface SlideView : UIView {
   ROOT_iOS::Pad *pad;
}

+ (CGSize) slideSize;
+ (CGRect) slideFrame;

- (id) initWithFrame : (CGRect)rect andPad : (ROOT_iOS::Pad *)pad;

@end
