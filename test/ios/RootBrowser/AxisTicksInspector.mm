#import <string.h>

#import "ROOTObjectController.h"
#import "AxisTicksInspector.h"

#import "TObject.h"
#import "TAxis.h"

static const float tickLengthStep = 0.01f;
static const float maxTickLength = 1.f;
static const float minTickLength = -1.f;

@implementation AxisTicksInspector

//_________________________________________________________________
- (id)initWithNibName:(NSString *)nibNameOrNil bundle:(NSBundle *)nibBundleOrNil
{
   self = [super initWithNibName:nibNameOrNil bundle:nibBundleOrNil];
   
   [self view];
   
   if (self) {
      // Custom initialization
   }

   return self;
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

#pragma mark - Inspector's protocol.

//_________________________________________________________________
- (void) setTicksWidgets
{
//   object->SetNdivisions(int(primaryTicks.value), int(secondaryTicks.value), int(tertiaryTicks.value), 1);
//   [controller objectWasModifiedByEditor];

   const int nDivisions = object->GetNdivisions();
   //Hardcoded constants from TAttAxis, never defined as named values in ROOT.
   //"Formulas" from TAxisEditor.
   primaryTicks = nDivisions % 100;
   secondaryTicks = (nDivisions / 100) % 100;
   tertiaryTicks = (nDivisions / 10000) % 100;
   
   primLabel.text = [NSString stringWithFormat : @"%u", primaryTicks];
   secLabel.text = [NSString stringWithFormat : @"%u", secondaryTicks];
   terLabel.text = [NSString stringWithFormat : @"%u", tertiaryTicks];
   
   tickLength = object->GetTickLength();
   tickLengthLabel.text = [NSString stringWithFormat : @"%.2f", object->GetTickLength()];
}

//_________________________________________________________________
- (void) setupInspector
{
   const char *option = object->GetTicks();
   
   if (!strcmp("+-", option))
      [ticksNegPos setSelectedSegmentIndex : 1];
   else 
      [ticksNegPos setSelectedSegmentIndex : 0];

   [self setTicksWidgets];
}


//_________________________________________________________________
- (void) setROOTObject : (TObject *)o
{
   //The result of a cast is not checked here, it's done on top level.
   object = dynamic_cast<TAxis *>(o);

   [self setupInspector];
}

//_________________________________________________________________
- (void) setROOTObjectController:(ROOTObjectController *)c
{
   controller = c;
}

//_________________________________________________________________
- (void) back
{
   [self.navigationController popViewControllerAnimated : YES];
}

//_________________________________________________________________
- (IBAction) ticksNegPos
{
   if (ticksNegPos.selectedSegmentIndex == 0)
      object->SetTicks("");
   else
      object->SetTicks("+-");

   [controller objectWasModifiedUpdateSelection : NO];
}

//_________________________________________________________________
- (void) setTicks
{
   object->SetNdivisions(primaryTicks, secondaryTicks, tertiaryTicks);
}

//_________________________________________________________________
- (IBAction) plusTick : (UIButton *)sender
{
   if (sender == plusLengthBtn) {
      if (tickLength + tickLengthStep < maxTickLength) {
         tickLength += tickLengthStep;
         tickLengthLabel.text = [NSString stringWithFormat:@"%.2f", tickLength];
         object->SetTickLength(tickLength);
         [controller objectWasModifiedUpdateSelection : NO];
      }
      return;
   }
   
   UILabel *labelToModify = 0;
   unsigned n = 0;

   if (sender == plusPrim) {
      labelToModify = primLabel;
      if (primaryTicks < 99)
         n = ++primaryTicks;
      else
         return;
   } else if (sender == plusSec) {
      labelToModify = secLabel;
      if (secondaryTicks < 99)
         n = ++secondaryTicks;
      else
         return;
   } else {
      labelToModify = terLabel;
      if (tertiaryTicks < 99)
         n = ++tertiaryTicks;
      else
         return;
   }
   
   labelToModify.text = [NSString stringWithFormat : @"%u", n];
   [self setTicks];
   [controller objectWasModifiedUpdateSelection : NO];
}

//_________________________________________________________________
- (IBAction) minusTick :(UIButton *)sender
{
   if (sender == minusLengthBtn) {
      if (tickLength - tickLengthStep > minTickLength) {
         tickLength -= tickLengthStep;
         tickLengthLabel.text = [NSString stringWithFormat:@"%.2g", tickLength];
         object->SetTickLength(tickLength);
         [controller objectWasModifiedUpdateSelection : NO];
      }
      return;
   }

   UILabel *labelToModify = 0;
   unsigned n = 0;

   if (sender == minusPrim) {
      labelToModify = primLabel;
      if (primaryTicks > 0)
         n = --primaryTicks;
      else
         return;
   } else if (sender == minusSec) {
      labelToModify = secLabel;
      if (secondaryTicks > 0)
         n = --secondaryTicks;
      else
         return;
   } else {
      labelToModify = terLabel;
      if (tertiaryTicks > 0)
         n = --tertiaryTicks;
      else
         return;
   }
   
   labelToModify.text = [NSString stringWithFormat : @"%u", n];
   [self setTicks];
   [controller objectWasModifiedUpdateSelection : NO];
}


@end
