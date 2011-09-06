#import "ROOTObjectController.h"
#import "PadTicksGridInspector.h"

#import "TVirtualPad.h"
#import "TObject.h"

@implementation PadTicksGridInspector

//_________________________________________________________________
- (id)initWithNibName : (NSString *)nibNameOrNil bundle : (NSBundle *)nibBundleOrNil
{
   self = [super initWithNibName : nibNameOrNil bundle : nibBundleOrNil];

   [self view];

   if (self) {
   }
   
   return self;
}

//_________________________________________________________________
- (void)dealloc
{
   [super dealloc];
}

//_________________________________________________________________
- (void) setROOTObject : (TObject *)o
{
   object = static_cast<TVirtualPad *>(o);
   
   //I do not check the result of cast here, it's done on upper level.
   gridX.on = object->GetGridx();
   gridY.on = object->GetGridy();
   ticksX.on = object->GetTickx();
   ticksY.on = object->GetTicky();
}

//_________________________________________________________________
- (void) setROOTObjectController : (ROOTObjectController *)c
{
   controller = c;
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

#pragma mark - Inspector's actions.

//_________________________________________________________________
- (IBAction) gridActivated : (UISwitch *) g
{
   if (g == gridX)
      object->SetGridx(g.on);
   else if (g == gridY)
      object->SetGridy(g.on);
   
   [controller objectWasModifiedByEditor];
}

//_________________________________________________________________
- (IBAction) ticksActivated : (UISwitch *) t
{
   if (t == ticksX)
      object->SetTickx(t.on);
   else if (t == ticksY)
      object->SetTicky(t.on);
   
   [controller objectWasModifiedByEditor];
}

//_________________________________________________________________
- (IBAction) back
{
   [self.navigationController popViewControllerAnimated : YES];
}

@end