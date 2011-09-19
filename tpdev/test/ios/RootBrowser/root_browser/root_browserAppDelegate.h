#import <UIKit/UIKit.h>

@class RootFileController;

@interface root_browserAppDelegate : NSObject <UIApplicationDelegate> {

   RootFileController *rootController;
   UINavigationController *navigationController;
}

@property (nonatomic, retain) IBOutlet UIWindow *window;

@end
