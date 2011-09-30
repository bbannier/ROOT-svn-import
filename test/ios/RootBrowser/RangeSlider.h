//The range slider by Mal Curtis (http://buildmobile.com). Formatting and names modified by tpochep.

#import <UIKit/UIKit.h>

@interface RangeSlider : UIControl {
@private
   float minimumValue;
   float maximumValue;
   float minimumRange;
   float selectedMinimumValue;
   float selectedMaximumValue;

   float padding;
    
   BOOL maxThumbOn;
   BOOL minThumbOn;
    
   UIImageView *minThumb;
   UIImageView *maxThumb;
   UIImageView *track;
}

@property(nonatomic) float minimumValue;
@property(nonatomic) float maximumValue;
@property(nonatomic) float minimumRange;
@property(nonatomic) float selectedMinimumValue;
@property(nonatomic) float selectedMaximumValue;

- (id) initWithFrame : (CGRect)frame min : (float) min max : (float) max selectedMin : (float)sMin selectedMax : (float) sMax;

- (CGFloat) minThumbX;
- (CGFloat) maxThumbX;

@end
