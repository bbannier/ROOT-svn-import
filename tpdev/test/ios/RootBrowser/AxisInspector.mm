#import "AxisLabelsInspector.h"
#import "AxisTitleInspector.h"
#import "AxisColorInspector.h"
#import "AxisTicksInspector.h"
#import "AxisInspector.h"

@implementation AxisInspector

@synthesize tabBar;

//____________________________________________________________________________________________________
+ (CGRect) inspectorFrame
{
   return CGRectMake(0.f, 0.f, 250.f, 400.f);
}

//____________________________________________________________________________________________________
- (id)initWithNibName:(NSString *)nibNameOrNil bundle:(NSBundle *)nibBundleOrNil
{
   self = [super initWithNibName:nibNameOrNil bundle:nibBundleOrNil];

   [self view];

   if (self) {
      tabBar.selectedItem = [[tabBar items] objectAtIndex : 0];
   
   }
    
   return self;
}

//____________________________________________________________________________________________________
- (void) dealloc
{
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

#pragma mark - ObjectInspectorComponent's protocol.
//____________________________________________________________________________________________________
- (void) setROOTObjectController : (ROOTObjectController *)c
{
   controller = c;
}

//____________________________________________________________________________________________________
- (void) setROOTObject : (TObject *)o
{
   object = o;
}

//____________________________________________________________________________________________________
- (NSString *) getComponentName
{
   return @"Axis attributes";
}

//____________________________________________________________________________________________________
- (void) resetInspector
{
}

//____________________________________________________________________________________________________
- (IBAction) showColorInspector
{
   AxisColorInspector *colorInspector = [[AxisColorInspector alloc] initWithNibName : @"AxisColorInspector" bundle : nil mode : ROOT_IOSObjectInspector::acimAxisColor];

   [colorInspector setROOTObjectController : controller];
   [colorInspector setROOTObject : object];
   
   [self.navigationController pushViewController : colorInspector animated : YES];
   
   [colorInspector release];
}

//____________________________________________________________________________________________________
- (IBAction) showTicksInspector
{
   AxisTicksInspector *ticksInspector = [[AxisTicksInspector alloc] initWithNibName : @"AxisTicksInspector" bundle : nil];

   [ticksInspector setROOTObjectController : controller];
   [ticksInspector setROOTObject : object];
   
   [self.navigationController pushViewController : ticksInspector animated : YES];
   [ticksInspector release];
}

//____________________________________________________________________________________________________
- (IBAction) showAxisTitleInspector
{
   AxisTitleInspector *titleInspector = [[AxisTitleInspector alloc] initWithNibName : @"AxisTitleInspector" bundle : nil];

   [titleInspector setROOTObjectController : controller];
   [titleInspector setROOTObject : object];

   [self.navigationController pushViewController : titleInspector animated : YES];
   [titleInspector release];
}

//____________________________________________________________________________________________________
- (IBAction) showAxisLabelsInspector
{
   AxisLabelsInspector *labelsInspector = [[AxisLabelsInspector alloc] initWithNibName : @"AxisLabelsInspector" bundle : nil];

   [labelsInspector setROOTObjectController : controller];
   [labelsInspector setROOTObject : object];
   
   [self.navigationController pushViewController : labelsInspector animated : YES];
   [labelsInspector release];
}

#pragma mark - Tabbar delegate.

//____________________________________________________________________________________________________
- (void) tabBar : (UITabBar *) tb didSelectItem : (UITabBarItem *)item
{
/*   if (item.tag == 1)
      [self showTicksAndGridInspector];
   else if (item.tag == 2)
      [self showLogScaleInspector];*/
}


@end
