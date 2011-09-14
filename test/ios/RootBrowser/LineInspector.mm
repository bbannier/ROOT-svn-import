#import "LineColorWidthInspector.h"
#import "LineStyleInspector.h"
#import "LineInspector.h"

static const CGRect inspectorFrame = CGRectMake(0.f, 0.f, 250.f, 300.f);

@implementation LineInspector

//____________________________________________________________________________________________________
+ (CGRect) inspectorFrame
{
   return CGRectMake(0.f, 0.f, 250.f, 330.f);
}

//____________________________________________________________________________________________________
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

//____________________________________________________________________________________________________
- (void) dealloc
{
   [colorWidthInspector release];
   [styleInspector release];

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
   return @"Line attributes";
}

//____________________________________________________________________________________________________
- (IBAction) showColorWidthComponent
{
   [colorWidthInspector setROOTObjectController : controller];
   [colorWidthInspector setROOTObject : object];
   
   [self.navigationController pushViewController : colorWidthInspector animated : YES];
}

//____________________________________________________________________________________________________
- (IBAction) showStyleComponent
{
   [styleInspector setROOTObjectController : controller];
   [styleInspector setROOTObject : object];
   
   [self.navigationController pushViewController : styleInspector animated : YES];
}

@end
