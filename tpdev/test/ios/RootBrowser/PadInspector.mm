#import "PadTicksGridInspector.h"
#import "PadLogScaleInspector.h"
#import "PadInspector.h"

@implementation PadInspector

//_________________________________________________________________
+ (CGRect) inspectorFrame
{
   return CGRectMake(0.f, 0.f, 250.f, 250.f);
}

//_________________________________________________________________
- (id)initWithNibName : (NSString *)nibNameOrNil bundle : (NSBundle *)nibBundleOrNil
{
   self = [super initWithNibName:nibNameOrNil bundle:nibBundleOrNil];
    
   [self view];
    
   if (self) {
      //Load inspectors from nib files.
      gridInspector = [[PadTicksGridInspector alloc] initWithNibName : @"PadTicksGridInspector" bundle : nil];
      logScaleInspector = [[PadLogScaleInspector alloc] initWithNibName : @"PadLogScaleInspector" bundle : nil];
   }

   return self;
}

//_________________________________________________________________
- (void) dealloc
{
   [gridInspector release];
   [logScaleInspector release];

   [super dealloc];
}

//_________________________________________________________________
- (void)didReceiveMemoryWarning
{
   // Releases the view if it doesn't have a superview.
   [super didReceiveMemoryWarning];
   // Release any cached data, images, etc that aren't in use.
}

#pragma mark - View lifecycle

//_________________________________________________________________
- (void)viewDidLoad
{
   [super viewDidLoad];
   // Do any additional setup after loading the view from its nib.
}

//_________________________________________________________________
- (void)viewDidUnload
{
   [super viewDidUnload];
   // Release any retained subviews of the main view.
   // e.g. self.myOutlet = nil;
}

//_________________________________________________________________
- (BOOL)shouldAutorotateToInterfaceOrientation:(UIInterfaceOrientation)interfaceOrientation
{
   // Return YES for supported orientations
	return YES;
}

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
   return @"Pad's attributes";
}

//_________________________________________________________________
- (IBAction) showTicksAndGridInspector
{
   [gridInspector setROOTObjectController : controller];
   [gridInspector setROOTObject : object];
   
   [self.navigationController pushViewController : gridInspector animated : YES];
}

//_________________________________________________________________
- (IBAction) showLogScaleInspector
{
   [logScaleInspector setROOTObjectController : controller];
   [logScaleInspector setROOTObject : object];
   
   [self.navigationController pushViewController : logScaleInspector animated : YES];
}

@end
