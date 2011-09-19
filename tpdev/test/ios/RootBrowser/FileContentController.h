#import <UIKit/UIKit.h>

namespace ROOT_iOS {

class FileContainer;

}

class TObject;

@class ObjectShortcut;

@interface FileContentController : UIViewController {   
   NSMutableArray *objectShortcuts;
   ROOT_iOS::FileContainer *fileContainer;
   IBOutlet UIScrollView *scrollView;
}

@property (nonatomic, retain) UIScrollView *scrollView;
@property (nonatomic, readonly) ROOT_iOS::FileContainer *fileContainer;

- (void) activateForFile : (ROOT_iOS::FileContainer *)container;
- (void) selectObjectFromFile : (ObjectShortcut *)obj;

@end
