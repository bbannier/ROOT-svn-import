#import <UIKit/UIKit.h>

@class PadImageView;

namespace ROOT_iOS {

class Pad;

}

class TObject;

@interface PadImageScrollView : UIScrollView <UIScrollViewDelegate> {
@private
   UIImage *padImage;
   ROOT_iOS::Pad *pad;
   
   PadImageView *nestedView;
}

@property (assign) UIImage *padImage;

+ (CGRect) defaultImageFrame;

- (id) initWithFrame : (CGRect)frame;
- (void) setPad : (ROOT_iOS::Pad *)pad;
- (void) setPad : (ROOT_iOS::Pad *)pad andImage : (UIImage *)image;
- (void) resetToFrame : (CGRect)frame;

@end
