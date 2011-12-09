#import "RootQuartzWindow.h"
#import "RootQuartzView.h"

@implementation RootQuartzWindow

@synthesize fTopLevelView;

//______________________________________________________________________________
- (void) setFTopLevelView : (RootQuartzView *)view
{
   [self setContentView : view];
   fTopLevelView = view;
}

//______________________________________________________________________________
- (void) addChildView : (RootQuartzView *)view
{
   [fTopLevelView addSubview : view];
}

@end