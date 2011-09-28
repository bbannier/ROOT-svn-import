#import "LineColorWidthInspector.h"
#import "LineStyleInspector.h"
#import "LineInspector.h"


static const CGFloat totalHeight = 330.f;
static const CGFloat tabBarHeight = 49.f;
static const CGRect nestedComponentFrame = CGRectMake(0.f, tabBarHeight, 250.f, totalHeight - tabBarHeight);

@implementation LineInspector

@synthesize tabBar;

//____________________________________________________________________________________________________
+ (CGRect) inspectorFrame
{
   return CGRectMake(0.f, 0.f, 250.f, totalHeight);
}

//____________________________________________________________________________________________________
- (id)initWithNibName : (NSString *)nibNameOrNil bundle:(NSBundle *)nibBundleOrNil
{
   self = [super initWithNibName : nibNameOrNil bundle : nibBundleOrNil];
    
   [self view];
    
   if (self) {
      //Load inspectors from nib files.
      colorWidthInspector = [[LineColorWidthInspector alloc] initWithNibName : @"LineColorWidthInspector" bundle : nil];
      colorWidthInspector.view.frame = nestedComponentFrame;
      colorWidthInspector.view.hidden = NO;
      
      styleInspector = [[LineStyleInspector alloc] initWithNibName : @"LineStyleInspector" bundle : nil];
      styleInspector.view.frame = nestedComponentFrame;
      styleInspector.view.hidden = YES;

      [self.view addSubview : colorWidthInspector.view];
      [self.view addSubview : styleInspector.view];
      
      tabBar.selectedItem = [[tabBar items] objectAtIndex : 0];
   }
   return self;
}

//____________________________________________________________________________________________________
- (void) dealloc
{
   [colorWidthInspector release];
   [styleInspector release];
   self.tabBar = nil;

   [super dealloc];
}

//____________________________________________________________________________________________________
- (void)didReceiveMemoryWarning
{
   // Releases the view if it doesn't have a superview.
   [super didReceiveMemoryWarning];    
   // Release any cached data, images, etc that aren't in use.
}

#pragma mark - View lifecycle

//____________________________________________________________________________________________________
- (void)viewDidLoad
{
   [super viewDidLoad];
   // Do any additional setup after loading the view from its nib.
}

//____________________________________________________________________________________________________
- (void)viewDidUnload
{
   [super viewDidUnload];
   // Release any retained subviews of the main view.
   // e.g. self.myOutlet = nil;
}

//____________________________________________________________________________________________________
- (BOOL)shouldAutorotateToInterfaceOrientation:(UIInterfaceOrientation)interfaceOrientation
{
   // Return YES for supported orientations
	return YES;
}

//____________________________________________________________________________________________________
- (void) setROOTObjectController : (ROOTObjectController *)c
{
   controller = c;
   [colorWidthInspector setROOTObjectController : c];
   [styleInspector setROOTObjectController : c];
}

//____________________________________________________________________________________________________
- (void) setROOTObject : (TObject *)o
{
   object = o;
   [colorWidthInspector setROOTObject : o];   
   [styleInspector setROOTObject : o];
}

//____________________________________________________________________________________________________
- (NSString *) getComponentName
{
   return @"Line attributes";
}

//____________________________________________________________________________________________________
- (void) resetInspector
{
   tabBar.selectedItem = [[tabBar items] objectAtIndex : 0];
   [self showColorWidthComponent];
}

//____________________________________________________________________________________________________
- (IBAction) showColorWidthComponent
{
   colorWidthInspector.view.hidden = NO;
   styleInspector.view.hidden = YES;
}

//____________________________________________________________________________________________________
- (IBAction) showStyleComponent
{
   styleInspector.view.hidden = NO;
   colorWidthInspector.view.hidden = YES;
}

#pragma mark - UITabBar delegate.

//____________________________________________________________________________________________________
- (void) tabBar : (UITabBar *) tb didSelectItem : (UITabBarItem *)item
{
   if (item.tag == 1)
      [self showColorWidthComponent];
   else if (item.tag == 2)
      [self showStyleComponent];
}

@end
