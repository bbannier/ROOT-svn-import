#import "LineColorWidthInspector.h"
#import "ROOTObjectController.h"
#import "LineWidthCell.h"
#import "ColorCell.h"
#import "Constants.h"

//C++ (ROOT) imports.
#import "TAttLine.h"
#import "TObject.h"

static const CGRect cellFrame = CGRectMake(0.f, 0.f, 80.f, 44.f);

@implementation LineColorWidthInspector

//_________________________________________________________________
- (id)initWithNibName : (NSString *)nibNameOrNil bundle : (NSBundle *)nibBundleOrNil
{
   using namespace ROOT_IOSBrowser;

   self = [super initWithNibName : nibNameOrNil bundle : nibBundleOrNil];
   
   [self view];
   
   if (self) {
      // Custom initialization
      //Two mutable arrays with views for "Line color and width" picker.
      lineColors = [[NSMutableArray alloc] init];
      for (unsigned i = 0; i < nROOTDefaultColors; ++i) {
         ColorCell *newCell = [[ColorCell alloc] initWithFrame : cellFrame];
         [newCell setRGB : predefinedFillColors[i]];
         [lineColors addObject : newCell];
         [newCell release];
      }
      
      lineWidths = [[NSMutableArray alloc] init];
      for (unsigned i = 0; i < 15; ++i) {
         LineWidthCell * newCell = [[LineWidthCell alloc] initWithFrame : cellFrame width : i + 1];
         [lineWidths addObject : newCell];
         [newCell release];
      }
   }

   return self;
}

//_________________________________________________________________
- (void) dealloc
{
   [lineColors release];
   [lineWidths release];

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

#pragma main - Navigation

//_________________________________________________________________
- (IBAction) back
{
   [self.navigationController popViewControllerAnimated : YES];
}

#pragma mark - ObjectInspectorComponent protocol

//_________________________________________________________________
- (void) setROOTObjectController : (ROOTObjectController *) c
{
   controller = c;
}

//_________________________________________________________________
- (void) setROOTObject : (TObject *) obj
{
   using namespace ROOT_IOSBrowser;

   //I do not check the result of dynamic_cast here. This is done at upper level.
   object = dynamic_cast<TAttLine *>(obj);

   //Extract line color.
   //The same predefined 16 colors as with fill color.
   unsigned pickerRow = 0;
   const Color_t colorIndex = object->GetLineColor();
   
   for (unsigned i = 0; i < nROOTDefaultColors; ++i) {
      if (colorIndex == colorIndices[i]) {
         pickerRow = i;
         break;
      }
   }
   
   [linePicker selectRow : pickerRow inComponent : 0 animated : NO];

   //Line width: in ROOT it can be 0, can be negative.
   //Editor shows line widths in [1:15] range, so do I.
   {
      int pickerRow = (int)object->GetLineWidth();
      if (pickerRow < 1 || pickerRow > 15)
         pickerRow = 0;
      else
         pickerRow -= 1;
      
      [linePicker selectRow : pickerRow inComponent : 1 animated : NO];
   }
}

#pragma mark - Color/Width/Style picker's dataSource.

//_________________________________________________________________
- (CGFloat)pickerView : (UIPickerView *)pickerView widthForComponent : (NSInteger)component
{
   return cellFrame.size.width;
}

//_________________________________________________________________
- (CGFloat)pickerView : (UIPickerView *)pickerView rowHeightForComponent : (NSInteger)component
{
   return cellFrame.size.height;
}

//_________________________________________________________________
- (NSInteger)pickerView : (UIPickerView *)pickerView numberOfRowsInComponent : (NSInteger)component
{
   if (!component) {
      return [lineColors count];
   } else {
      return [lineWidths count];
   }
}

//_________________________________________________________________
- (NSInteger)numberOfComponentsInPickerView : (UIPickerView *)pickerView
{
   return 2;
}

#pragma mark color/pattern picker's delegate.

// tell the picker which view to use for a given component and row, we have an array of views to show
//_________________________________________________________________
- (UIView *)pickerView : (UIPickerView *)pickerView viewForRow : (NSInteger)row forComponent : (NSInteger)component reusingView : (UIView *)view
{
   if (component == 0) {
      return [lineColors objectAtIndex : row];
   } else {
      return [lineWidths objectAtIndex : row];
   }
}

//_________________________________________________________________
- (void)pickerView : (UIPickerView *)thePickerView didSelectRow : (NSInteger)row inComponent : (NSInteger)component
{
   if (!component) {
      const unsigned colorIndex = ROOT_IOSBrowser::colorIndices[row];
      object->SetLineColor(colorIndex);
   } else {
      const unsigned width = row + 1;
      object->SetLineWidth(width);
   }
   
   [controller objectWasModifiedUpdateSelection : NO];
}

@end
