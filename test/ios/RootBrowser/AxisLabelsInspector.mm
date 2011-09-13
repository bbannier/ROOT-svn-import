#import "ROOTObjectController.h"
#import "AxisLabelsInspector.h"
#import "AxisColorInspector.h"
#import "AxisFontInspector.h"

#import "TObject.h"
#import "TAxis.h"

static const float sizeStep = 0.01f;
static const float minSize = 0.f;
static const float maxSize = 1.f;

static const float offsetStep = 0.001f;
static const float minOffset = -1.f;
static const float maxOffset = 1.f;

@implementation AxisLabelsInspector

//_________________________________________________________________
- (id)initWithNibName:(NSString *)nibNameOrNil bundle:(NSBundle *)nibBundleOrNil
{
   self = [super initWithNibName:nibNameOrNil bundle:nibBundleOrNil];
   
   [self view];
   
   if (self) {
      colorInspector = [[AxisColorInspector alloc] initWithNibName : @"AxisColorInspector" bundle : nil mode : ROOT_IOSObjectInspector::acimLabelColor];
      fontInspector = [[AxisFontInspector alloc] initWithNibName : @"AxisFontInspector" mode : ROOT_IOSObjectInspector::afimLabelFont];
   }

   return self;
}

//_________________________________________________________________
- (void) dealloc
{
   [colorInspector release];
   [fontInspector release];
   
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
   
   sizeLabel.text = [NSString stringWithFormat : @"%.2f", object->GetLabelSize()];
   offsetLabel.text = [NSString stringWithFormat : @"%.3f", object->GetLabelOffset()];
   
   noExp.on = object->GetNoExponent();
}

//_________________________________________________________________
- (void) showLabelFontInspector
{
   [fontInspector setROOTObjectController : controller];
   [fontInspector setROOTObject : object];
   
   [self.navigationController pushViewController : fontInspector animated : YES];
}

//_________________________________________________________________
- (void) showLabelColorInspector
{
   [colorInspector setROOTObjectController : controller];
   [colorInspector setROOTObject : object];
   
   [self.navigationController pushViewController : colorInspector animated : YES];   
}

//_________________________________________________________________
- (IBAction) plusBtn : (UIButton *)sender
{
   if (sender == plusSize) {
      if (object->GetLabelSize() + sizeStep > maxSize)
         return;
      
      sizeLabel.text = [NSString stringWithFormat : @"%.2f", object->GetLabelSize() + sizeStep];
      object->SetLabelSize(object->GetLabelSize() + sizeStep);
   } else if (sender == plusOffset) {
      if (object->GetLabelOffset() + offsetStep > maxOffset)
         return;
      
      offsetLabel.text = [NSString stringWithFormat : @"%.3f", object->GetLabelOffset() + offsetStep];
      object->SetLabelOffset(object->GetLabelOffset() + offsetStep);
   }
   
   [controller objectWasModifiedByEditor];
}

//_________________________________________________________________
- (IBAction) minusBtn : (UIButton *)sender
{
   if (sender == minusSize) {
      if (object->GetLabelSize() - sizeStep < minSize)
         return;
      
      sizeLabel.text = [NSString stringWithFormat : @"%.2f", object->GetLabelSize() - sizeStep];
      object->SetLabelSize(object->GetLabelSize() - sizeStep);
   } else if (sender == minusOffset) {
      if (object->GetLabelOffset() - offsetStep < minOffset)
         return;
      
      offsetLabel.text = [NSString stringWithFormat : @"%.3f", object->GetLabelOffset() - offsetStep];
      object->SetLabelOffset(object->GetLabelOffset() - offsetStep);
   }
   
   [controller objectWasModifiedByEditor];
}

//_________________________________________________________________
- (IBAction) noExp
{
   object->SetNoExponent(noExp.on);
   [controller objectWasModifiedByEditor];
}

//_________________________________________________________________
- (IBAction) back
{
   [self.navigationController popViewControllerAnimated : YES];
}

@end
