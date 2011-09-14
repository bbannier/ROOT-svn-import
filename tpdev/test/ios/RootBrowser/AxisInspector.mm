#import "AxisLabelsInspector.h"
#import "AxisTitleInspector.h"
#import "AxisColorInspector.h"
#import "AxisTicksInspector.h"
#import "AxisInspector.h"

@implementation AxisInspector

//____________________________________________________________________________________________________
+ (CGRect) inspectorFrame
{
   return CGRectMake(0.f, 0.f, 250.f, 400.f);
}

//____________________________________________________________________________________________________
- (id)initWithNibName:(NSString *)nibNameOrNil bundle:(NSBundle *)nibBundleOrNil
{
   self = [super initWithNibName:nibNameOrNil bundle:nibBundleOrNil];
   if (self) {
      colorInspector = [[AxisColorInspector alloc] initWithNibName : @"AxisColorInspector" bundle : nil mode : ROOT_IOSObjectInspector::acimAxisColor];
      ticksInspector = [[AxisTicksInspector alloc] initWithNibName : @"AxisTicksInspector" bundle : nil];
      
      //titleInspector.
      titleInspector = [[AxisTitleInspector alloc] initWithNibName : @"AxisTitleInspector" bundle : nil];
      labelsInspector = [[AxisLabelsInspector alloc] initWithNibName : @"AxisLabelsInspector" bundle : nil];
   }
    
   return self;
}

//____________________________________________________________________________________________________
- (void) dealloc
{
   [colorInspector release];
   [titleInspector release];
   [labelsInspector release];
   [ticksInspector release];
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
//   [titleInspector resetInspector];
//   [labelsInspector resetInspector];
}

//____________________________________________________________________________________________________
- (IBAction) showColorInspector
{
   [colorInspector setROOTObjectController : controller];
   [colorInspector setROOTObject : object];
   
   [self.navigationController pushViewController : colorInspector animated : YES];
}

//____________________________________________________________________________________________________
- (IBAction) showTicksInspector
{
   [ticksInspector setROOTObjectController : controller];
   [ticksInspector setROOTObject : object];
   
   [self.navigationController pushViewController : ticksInspector animated : YES];
}

//____________________________________________________________________________________________________
- (IBAction) showAxisTitleInspector
{
   [titleInspector setROOTObjectController : controller];
   [titleInspector setROOTObject : object];

   [self.navigationController pushViewController : titleInspector animated : YES];
}

//____________________________________________________________________________________________________
- (IBAction) showAxisLabelsInspector
{
   [labelsInspector setROOTObjectController : controller];
   [labelsInspector setROOTObject : object];
   
   [self.navigationController pushViewController : labelsInspector animated : YES];
}

@end
