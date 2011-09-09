#import "ROOTObjectController.h"
#import "AxisTitleInspector.h"
#import "AxisColorInspector.h"

//C++ (ROOT) imports.
#import "TObject.h"
#import "TAxis.h"


@implementation AxisTitleInspector

//_________________________________________________________________
+ (CGRect) inspectorFrame
{
   return CGRectMake(0.f, 0.f, 250.f, 400.f);
}

//_________________________________________________________________
- (id)initWithNibName : (NSString *)nibNameOrNil bundle : (NSBundle *)nibBundleOrNil
{
   self = [super initWithNibName : nibNameOrNil bundle : nibBundleOrNil];
   
   [self view];
   
   if (self) {
      colorInspector = [[AxisColorInspector alloc] initWithNibName : @"AxisColorInspector" bundle : nil mode : ROOT_IOSObjectInspector::acimTitleColor];
   }

   return self;
}

//_________________________________________________________________
- (void) dealloc
{
   [colorInspector release];
   
   [super release];
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
   object = dynamic_cast<TAxis *>(o);

   const char *axisTitle = object->GetTitle();
   if (!axisTitle || !*axisTitle)
      titleField.text = @"";
   else
      titleField.text = [NSString stringWithFormat : @"%s", axisTitle];
      
   centered.on = object->GetCenterTitle();
   rotated.on = object->GetRotateTitle();
}

//_________________________________________________________________
- (void) back
{
   [self.navigationController popViewControllerAnimated : YES];   
}

//_________________________________________________________________
- (IBAction) showTitleFontInspector
{
}

//_________________________________________________________________
- (IBAction) showTitleColorInspector
{
   [colorInspector setROOTObjectController : controller];
   [colorInspector setROOTObject : object];
   
   [self.navigationController pushViewController : colorInspector animated : YES];   
}

//____________________________________________________________________________________________________
- (IBAction) textFieldDidEndOnExit : (id) sender
{
   object->SetTitle([titleField.text cStringUsingEncoding : [NSString defaultCStringEncoding]]);
   [controller objectWasModifiedByEditor];
}

//____________________________________________________________________________________________________
- (IBAction) textFieldEditingDidEnd : (id) sender
{
   [sender resignFirstResponder];
}

//____________________________________________________________________________________________________
- (IBAction) centerTitle
{
   object->CenterTitle(centered.on);
   [controller objectWasModifiedByEditor];
}

//____________________________________________________________________________________________________
- (IBAction) rotateTitle
{
   object->RotateTitle(rotated.on);
   [controller objectWasModifiedByEditor];
}

@end
