#import <UIKit/UIKit.h>


namespace ROOT_iOS {

class FileContainer;

}

@interface FileShortcut : UIView {
   UIViewController *controller;

   NSString *fileName;
   NSString *filePath;

   UIImage *filePictogram;
   UIImage *backgroundImage;
   
   ROOT_iOS::FileContainer *fileContainer;
}

@property (nonatomic, retain) NSString *fileName;
@property (nonatomic, retain) NSString *filePath;
@property (nonatomic, retain) NSString *errorMessage;

+ (CGFloat) iconWidth;
+ (CGFloat) iconHeight;

- (id) initWithFrame : (CGRect)frame controller : (UIViewController *)c filePath : (NSString *) path;

- (ROOT_iOS::FileContainer *) getFileContainer;

@end
