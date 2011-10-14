#import <UIKit/UIKit.h>

@class LineWidthCell;

@interface LineWidthPicker : UIView {
@private
   float lineWidth;
   LineWidthCell *lineWidthView;
   UIImage *backgroundImage;
}

- (void) incLineWidth;
- (void) decLineWidth;
- (float) getLineWidth;

@end
