#import <UIKit/UIKit.h>

namespace ROOT_iOS {

class FileContainer;
class Pad;

}

@interface SlideshowController : UIViewController <UIScrollViewDelegate> {
   ROOT_iOS::Pad *pad;

   UIImageView *padImageViews[3];
   UIImage *padImages[3];

   unsigned visiblePad;
   unsigned nCurrentObject;
   
   ROOT_iOS::FileContainer *fileContainer;
   
   IBOutlet UIView *parentView;
   IBOutlet UIView *shadowCaster;
   IBOutlet UIScrollView *scrollView;
	
	int prevIndex;
	int currIndex;
	int nextIndex;
}

- (id)initWithNibName : (NSString *)nibNameOrNil bundle : (NSBundle *)nibBundleOrNil fileContainer : (ROOT_iOS::FileContainer *)container;

@end
