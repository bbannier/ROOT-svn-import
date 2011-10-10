#import <UIKit/UIKit.h>

#import "TAttMarker.h"

@interface MarkerStyleCell : UIView {
@private
   UIImage *backgroundImage;
   EMarkerStyle markerStyle;
}

- (id) initWithFrame : (CGRect)frame andMarkerStyle : (EMarkerStyle)style;

@property (nonatomic, retain) UIImage *backgroundImage;

@end
