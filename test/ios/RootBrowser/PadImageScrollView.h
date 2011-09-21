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
   
   TObject *object;
   const char *drawOption;
   
   PadImageView *nestedView;
}

@property (assign) UIImage *padImage;

+ (CGRect) defaultImageFrame;

- (id) initWithFrame : (CGRect)frame andPad : (ROOT_iOS::Pad*)pad;
- (void) setObject : (TObject *)obj drawOption : (const char*)option;
- (void) setObject : (TObject *)obj drawOption : (const char *) opt andImage : (UIImage *)image;
- (void) resetToFrame : (CGRect)frame;

@end
