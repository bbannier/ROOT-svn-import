#import <UIKit/UIKit.h>

@class EditorView;

@interface EditorPlateView : UIView {
@private
   UIImage *plateImage;
   UILabel *editorLabel;
   
   UIImage *arrowImage;
   
   EditorView *topView;
}

@property (nonatomic, retain) NSString *editorName;
@property (nonatomic, retain) UIImageView *arrowImageView;

+ (CGFloat) plateHeight;

- (id) initWithFrame : (CGRect)frame editorName : (NSString *) name topView : (EditorView *) tv;

- (void) handleTap : (UITapGestureRecognizer *) tap;

@end
