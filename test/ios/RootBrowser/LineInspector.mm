#import "LineColorWidthInspector.h"
#import "LineStyleInspector.h"
#import "LineInspector.h"

static const CGRect inspectorFrame = CGRectMake(0.f, 0.f, 250.f, 300.f);

@implementation LineInspector

//_________________________________________________________________
- (id)initWithNibName : (NSString *)nibNameOrNil bundle:(NSBundle *)nibBundleOrNil
{
   self = [super initWithNibName:nibNameOrNil bundle:nibBundleOrNil];
    
   [self view];
    
   if (self) {
      //Load inspectors from nib files.
      colorWidthInspector = [[LineColorWidthInspector alloc] initWithNibName : @"LineColorWidthInspector" bundle : nil];
      colorWidthInspector.view.frame = inspectorFrame;
      
      styleInspector = [[LineStyleInspector alloc] initWithNibName : @"LineStyleInspector" bundle : nil];
      styleInspector.view.frame = inspectorFrame;
   }
   return self;
}

//_________________________________________________________________
- (void) dealloc
{
   [colorWidthInspector release];
   [styleInspector release];

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
   return @"Line's attributes";
}

//_________________________________________________________________
- (IBAction) showColorWidthComponent
{
   [colorWidthInspector setROOTObjectController : controller];
   [colorWidthInspector setROOTObject : object];
   
   [self.navigationController pushViewController : colorWidthInspector animated : YES];
}

//_________________________________________________________________
- (IBAction) showStyleComponent
{
   [styleInspector setROOTObjectController : controller];
   [styleInspector setROOTObject : object];
   
   [self.navigationController pushViewController : styleInspector animated : YES];
}

@end
