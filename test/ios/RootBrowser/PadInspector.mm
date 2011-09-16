#import "PadTicksGridInspector.h"
#import "PadLogScaleInspector.h"
#import "PadInspector.h"

@implementation PadInspector

//____________________________________________________________________________________________________
+ (CGRect) inspectorFrame
{
   return CGRectMake(0.f, 0.f, 250.f, 250.f);
}

//____________________________________________________________________________________________________
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

//____________________________________________________________________________________________________
- (void) dealloc
{
   [gridInspector release];
   [logScaleInspector release];

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
}

//____________________________________________________________________________________________________
- (void) setROOTObject : (TObject *)o
{
   object = o;
}

//____________________________________________________________________________________________________
- (NSString *) getComponentName
{
   return @"Pad attributes";
}

//____________________________________________________________________________________________________
- (IBAction) showTicksAndGridInspector
{
   [gridInspector setROOTObjectController : controller];
   [gridInspector setROOTObject : object];
   
   [self.navigationController pushViewController : gridInspector animated : YES];
}

//____________________________________________________________________________________________________
- (IBAction) showLogScaleInspector
{
   [logScaleInspector setROOTObjectController : controller];
   [logScaleInspector setROOTObject : object];
   
   [self.navigationController pushViewController : logScaleInspector animated : YES];
}

@end
