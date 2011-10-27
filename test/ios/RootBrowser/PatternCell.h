#import <UIKit/UIKit.h>

@interface PatternCell : UIView {
@private
   unsigned patternIndex;
   BOOL solid;
}

- (id) initWithFrame : (CGRect) frame andPattern : (unsigned) index;
- (void) setAsSolid;

@end
