#import "ROOTObjectController.h"
#import "AxisTitleInspector.h"
#import "AxisColorInspector.h"
#import "AxisFontInspector.h"

//C++ (ROOT) imports.
#import "TObject.h"
#import "TAxis.h"

static const float minTitleOffset = 0.1f;
static const float maxTitleOffset = 10.f;
static const float titleOffsetStep = 0.01f;

@implementation AxisTitleInspector

//____________________________________________________________________________________________________
+ (CGRect) inspectorFrame
{
   return CGRectMake(0.f, 0.f, 250.f, 400.f);
}

//____________________________________________________________________________________________________
- (id)initWithNibName : (NSString *)nibNameOrNil bundle : (NSBundle *)nibBundleOrNil
{
   self = [super initWithNibName : nibNameOrNil bundle : nibBundleOrNil];
   
   [self view];
   
   if (self) {
      colorInspector = [[AxisColorInspector alloc] initWithNibName : @"AxisColorInspector" bundle : nil mode : ROOT_IOSObjectInspector::acimTitleColor];
      fontInspector = [[AxisFontInspector alloc] initWithNibName : @"AxisFontInspector" mode : ROOT_IOSObjectInspector::afimTitleFont];
   }

   return self;
}

//____________________________________________________________________________________________________
- (void) dealloc
{
   [colorInspector release];
   [fontInspector release];
   
   [super release];
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
   object = dynamic_cast<TAxis *>(o);

   const char *axisTitle = object->GetTitle();
   if (!axisTitle || !*axisTitle)
      titleField.text = @"";
   else
      titleField.text = [NSString stringWithFormat : @"%s", axisTitle];
      
   centered.on = object->GetCenterTitle();
   rotated.on = object->GetRotateTitle();
   
   offset = object->GetTitleOffset();
   offsetLabel.text = [NSString stringWithFormat:@"%.2f", offset];
}

//____________________________________________________________________________________________________
- (void) back
{
   [self.navigationController popViewControllerAnimated : YES];   
}

//____________________________________________________________________________________________________
- (IBAction) showTitleFontInspector
{
   [fontInspector setROOTObjectController : controller];
   [fontInspector setROOTObject : object];
   
   [self.navigationController pushViewController : fontInspector animated : YES];
}

//____________________________________________________________________________________________________
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
   [controller objectWasModifiedUpdateSelection : NO];
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
   [controller objectWasModifiedUpdateSelection : NO];
}

//____________________________________________________________________________________________________
- (IBAction) rotateTitle
{
   object->RotateTitle(rotated.on);
   [controller objectWasModifiedUpdateSelection : NO];
}

//____________________________________________________________________________________________________
- (IBAction) plusOffset
{
   if (offset + titleOffsetStep > maxTitleOffset)
      return;
   
   offset += titleOffsetStep;
   offsetLabel.text = [NSString stringWithFormat:@"%.2f", offset];
   object->SetTitleOffset(offset);
   
   [controller objectWasModifiedUpdateSelection : NO];
}

//____________________________________________________________________________________________________
- (IBAction) minusOffset
{
   if (offset - titleOffsetStep < minTitleOffset)
      return;
   
   offset -= titleOffsetStep;
   offsetLabel.text = [NSString stringWithFormat:@"%.2f", offset];
   object->SetTitleOffset(offset);
   
   [controller objectWasModifiedUpdateSelection : NO];
}


@end
