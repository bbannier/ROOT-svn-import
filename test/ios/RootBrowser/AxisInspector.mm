#import "InspectorWithNavigation.h"
#import "AxisTitleInspector.h"
#import "AxisColorInspector.h"
#import "AxisTicksInspector.h"
#import "AxisInspector.h"

@implementation AxisInspector

+ (CGRect) inspectorFrame
{
   return CGRectMake(0.f, 0.f, 250.f, 400.f);
}

- (id)initWithNibName:(NSString *)nibNameOrNil bundle:(NSBundle *)nibBundleOrNil
{
   self = [super initWithNibName:nibNameOrNil bundle:nibBundleOrNil];
   if (self) {
      colorInspector = [[AxisColorInspector alloc] initWithNibName : @"AxisColorInspector" bundle : nil mode : ROOT_IOSObjectInspector::acimAxisColor];
      ticksInspector = [[AxisTicksInspector alloc] initWithNibName : @"AxisTicksInspector" bundle : nil];
      
      //titleInspector.
      titleInspector = [[AxisTitleInspector alloc] initWithNibName : @"AxisTitleInspector" bundle : nil];
//      titleInspector = [[InspectorWithNavigation alloc] initWithRootViewController : titleInspectorCompositor];
//      [titleInspectorCompositor release];
 //     titleInspector.view.frame = [AxisTitleInspector inspectorFrame];
      //labelInspector.
   }
    
   return self;
}

- (void) dealloc
{
   [colorInspector release];
   [titleInspector release];
   [ticksInspector release];
}

- (void)didReceiveMemoryWarning
{
    // Releases the view if it doesn't have a superview.
    [super didReceiveMemoryWarning];
    
    // Release any cached data, images, etc that aren't in use.
}

#pragma mark - View lifecycle

- (void)viewDidLoad
{
    [super viewDidLoad];
    // Do any additional setup after loading the view from its nib.
}

- (void)viewDidUnload
{
    [super viewDidUnload];
    // Release any retained subviews of the main view.
    // e.g. self.myOutlet = nil;
}

- (BOOL)shouldAutorotateToInterfaceOrientation:(UIInterfaceOrientation)interfaceOrientation
{
    // Return YES for supported orientations
	return YES;
}

#pragma mark - ObjectInspectorComponent's protocol.
//_________________________________________________________________
- (void) setROOTObjectController : (ROOTObjectController *)c
{
   controller = c;
}

//_________________________________________________________________
- (void) setROOTObject : (TObject *)o
{
   object = o;
}

//_________________________________________________________________
- (NSString *) getComponentName
{
   return @"Axis attributes";
}

//_________________________________________________________________
- (void) resetInspector
{
//   [titleInspector resetInspector];
//   [labelsInspector resetInspector];
}

//_________________________________________________________________
- (IBAction) showColorInspector
{
   [colorInspector setROOTObjectController : controller];
   [colorInspector setROOTObject : object];
   
   [self.navigationController pushViewController : colorInspector animated : YES];
}

//_________________________________________________________________
- (IBAction) showTicksInspector
{
   [ticksInspector setROOTObjectController : controller];
   [ticksInspector setROOTObject : object];
   
   [self.navigationController pushViewController : ticksInspector animated : YES];
}

//_________________________________________________________________
- (IBAction) showAxisTitleInspector
{
   [titleInspector setROOTObjectController : controller];
   [titleInspector setROOTObject : object];

   [self.navigationController pushViewController : titleInspector animated : YES];
}

//_________________________________________________________________
- (IBAction) showAxisLabelsInspector
{

}

@end
