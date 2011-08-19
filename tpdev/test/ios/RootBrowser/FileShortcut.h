#import <UIKit/UIKit.h>


@interface FileShortcut : UIView {
   UIViewController *controller;

   NSString *fileName;
   UIImage *filePictogram;
   UIImage *backgroundImage;
}

@property (nonatomic, retain) NSString *fileName;

+ (CGFloat) fileIconWidth;
+ (CGFloat) fileIconHeight;

- (id) initWithFrame : (CGRect)frame controller : (UIViewController *)c fileName : (NSString *)fileName contents : (unsigned)n;

@end
