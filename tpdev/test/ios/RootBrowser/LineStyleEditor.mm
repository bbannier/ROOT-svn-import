#import "ROOTObjectController.h"
#import "LineStyleEditor.h"
#import "LineWidthCell.h"
#import "LineStyleCell.h"
#import "Constants.h"
#import "ColorCell.h"

//C++ (ROOT) imports.
#import "TAttLine.h"
#import "TObject.h"

//TODO: remove line related constants to IOSLineStyles.h/*.cxx

static const CGFloat defaultCellH = 44.f;

static const CGRect smallCellFrame = CGRectMake(0.f, 0.f, 80.f, 44.f);
static const CGRect bigCellFrame = CGRectMake(0.f, 0.f, 180.f, 44.f);

@implementation LineStyleEditor

- (id)initWithNibName : (NSString *) nibNameOrNil bundle : (NSBundle *)nibBundleOrNil
{
   using namespace ROOT_IOSBrowser;

   self = [super initWithNibName : nibNameOrNil bundle : nibBundleOrNil];

   if (self) {
      //Two mutable arrays with views for "Line color and width" picker.
      lineColors = [[NSMutableArray alloc] init];
      for (unsigned i = 0; i < nROOTDefaultColors; ++i) {
         ColorCell *newCell = [[ColorCell alloc] initWithFrame : smallCellFrame];
         [newCell setRGB : predefinedFillColors[i]];
         [lineColors addObject : newCell];
         [newCell release];
      }
      
      lineWidths = [[NSMutableArray alloc] init];
      for (unsigned i = 0; i < 15; ++i) {
         LineWidthCell * newCell = [[LineWidthCell alloc] initWithFrame : smallCellFrame width : i + 1];
         [lineWidths addObject : newCell];
         [newCell release];
      }
      
      //Array with cells for "Line style" picker.
      lineStyles = [[NSMutableArray alloc] init];
      for (unsigned i = 0; i < 10; ++i) {
         LineStyleCell * newCell = [[LineStyleCell alloc] initWithFrame : bigCellFrame lineStyle : i + 1];
         [lineStyles addObject : newCell];
         [newCell release];
      }
   }

   return self;
}

- (void)dealloc
{
   [lineColors release];
   [lineWidths release];
   [lineStyles release];

   [super dealloc];
}

- (void)didReceiveMemoryWarning
{
   // Releases the view if it doesn't have a superview.
   [super didReceiveMemoryWarning];
   // Release any cached data, images, etc that aren't in use.
}

#pragma mark - View lifecycle

- (void)viewDidLoad
{
   [super viewDidLoad];
   // Do any additional setup after loading the view from its nib.
}

- (void)viewDidUnload
{
   [super viewDidUnload];
   // Release any retained subviews of the main view.
   // e.g. self.myOutlet = nil;
}

- (BOOL)shouldAutorotateToInterfaceOrientation:(UIInterfaceOrientation)interfaceOrientation
{
   // Return YES for supported orientations
	return YES;
}

#pragma mark - Color/Width/Style picker's dataSource.
//_________________________________________________________________
- (CGFloat)pickerView : (UIPickerView *)pickerView widthForComponent : (NSInteger)component
{
   if (pickerView == lineColorWidthPicker)
      return smallCellFrame.size.width;
   
   return bigCellFrame.size.width;
}

//_________________________________________________________________
- (CGFloat)pickerView : (UIPickerView *)pickerView rowHeightForComponent : (NSInteger)component
{
   return defaultCellH;
}

//_________________________________________________________________
- (NSInteger)pickerView : (UIPickerView *)pickerView numberOfRowsInComponent : (NSInteger)component
{
   if (pickerView == lineColorWidthPicker) {
      if (!component) {
         return [lineColors count];
      } else {
         return [lineWidths count];
      }
   } else if (pickerView == lineStylePicker) {
      return [lineStyles count];
   }

   return 0;
}

//_________________________________________________________________
- (NSInteger)numberOfComponentsInPickerView : (UIPickerView *)pickerView
{
   if (pickerView == lineColorWidthPicker)
      return 2;

	return 1;
}

#pragma mark color/pattern picker's delegate.

// tell the picker which view to use for a given component and row, we have an array of views to show
//_________________________________________________________________
- (UIView *)pickerView : (UIPickerView *)pickerView viewForRow : (NSInteger)row forComponent : (NSInteger)component reusingView : (UIView *)view
{
   if (pickerView == lineColorWidthPicker) {
      if (component == 0) {
         return [lineColors objectAtIndex : row];
      } else {
         return [lineWidths objectAtIndex : row];
      }
   }else if (pickerView == lineStylePicker)
      return [lineStyles objectAtIndex : row];

   return 0;
}

//_________________________________________________________________
- (void)pickerView : (UIPickerView *)thePickerView didSelectRow : (NSInteger)row inComponent : (NSInteger)component
{
/*   if (thePickerView == colorPicker)
      [self setNewColor : row];
   else if (thePickerView == patternPicker)
      [self setNewPattern : row];*/
//   NSLog(@"component %d row %d", component, row);
   if (thePickerView == lineColorWidthPicker) {
      if (!component) {
         const unsigned colorIndex = ROOT_IOSBrowser::colorIndices[row];
         object->SetLineColor(colorIndex);
      } else {
         const unsigned width = row + 1;
         object->SetLineWidth(width);
      }
   } else {
      const unsigned lineStyle = row + 1;
      object->SetLineStyle(lineStyle);
   }
   
   [controller objectWasModifiedByEditor];
}

//_________________________________________________________________
- (void) setController : (ROOTObjectController *) c
{
   controller = c;
}

//_________________________________________________________________
- (void) setObject : (TObject *) obj
{
   object = dynamic_cast<TAttLine *>(obj);

//   const Color_t currentColor = object->GetLineColor();
//   const Width_t currentWidth = object->GetLineWidth();
//   const Style_t currentStyle = object->GetLineStyle();
   
//   if (currentWidth >= 0 && currentWidth <= 15)
}

@end
