#import <UIKit/UIKit.h>

namespace ROOT_iOS {

class Pad;

}

@interface SelectionView : UIView {
   ROOT_iOS::Pad *pad;
   
   BOOL panActive;
   CGPoint panStart;
   CGPoint currentPanPoint;
   BOOL verticalDirection;
   
   CGPoint hitPoint;
   CGPoint offset;
}

@property (nonatomic, assign) BOOL panActive;
@property (nonatomic, assign) CGPoint panStart;
@property (nonatomic, assign) CGPoint currentPanPoint;
@property (nonatomic, assign) BOOL verticalDirection;

- (id)initWithFrame : (CGRect)frame withPad : (ROOT_iOS::Pad *) p;

@end
