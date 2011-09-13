#import <UIKit/UIKit.h>

namespace ROOT_iOS {

class Pad;

}

@interface SlideView : UIView {
   ROOT_iOS::Pad *pad;
}

- (id) initWithFrame : (CGRect) frame andPad : (ROOT_iOS::Pad *)pad;

@end
