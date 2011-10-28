#import <UIKit/UIKit.h>

@class FileShortcut;

@interface RootFileController : UIViewController <UINavigationControllerDelegate, UINavigationBarDelegate> {
@private
   __weak IBOutlet UIScrollView *scrollView;
   __weak IBOutlet UIView *fileOpenView;
   __weak IBOutlet UITextField *fileNameField;
}

- (void) fileWasSelected : (FileShortcut*) shortcut;
- (void) addFileShortcut : (NSString *) fileName;
- (void) hideFileOpenView;

- (IBAction) textFieldDidEndOnExit : (id) sender;
- (IBAction) textFieldEditingDidEnd : (id) sender;

@end
