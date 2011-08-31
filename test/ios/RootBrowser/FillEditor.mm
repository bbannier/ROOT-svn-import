#import "ROOTObjectController.h"
#import "PatternCell.h"
#import "FillEditor.h"
#import "Constants.h"
#import "ColorCell.h"

//C++ (ROOT) imports:
#import "IOSFillPatterns.h"
#import "TAttFill.h"
#import "TObject.h"


//Fill colors, visible in a ROOT's default color-picker (in editors).
//TODO: check, if in Obj-C++ constants have internal linkage.
static const CGFloat defaultCellW = 80.f;
static const CGFloat defaultCellH = 44.f;

@implementation FillEditor

//_________________________________________________________________
- (id)initWithNibName : (NSString *)nibNameOrNil bundle : (NSBundle *)nibBundleOrNil
{
   using namespace ROOT_IOSBrowser;

   self = [super initWithNibName:nibNameOrNil bundle:nibBundleOrNil];

   if (self) {
      const CGRect cellRect = CGRectMake(0.f, 0.f, defaultCellW, defaultCellH);
   
      colorCells = [[NSMutableArray alloc] init];
      
      for (unsigned i = 0; i < nROOTDefaultColors; ++i) {
         ColorCell * newCell = [[ColorCell alloc] initWithFrame : cellRect];
         [newCell setRGB : predefinedFillColors[i]];
         [colorCells addObject : newCell];
         [newCell release];
      }

      patternCells = [[NSMutableArray alloc] init];
      PatternCell *solidFill = [[PatternCell alloc] initWithFrame : cellRect andPattern : 0];
      [solidFill setAsSolid];
      [patternCells addObject : solidFill];
      [solidFill release];
      
      for (unsigned i = 0; i < ROOT_iOS::GraphicUtils::kPredefinedFillPatterns; ++i) {
         PatternCell *newCell = [[PatternCell alloc] initWithFrame : CGRectMake(0.f, 0.f, 80.f, 44.f) andPattern : i];
         [patternCells addObject : newCell];
         [newCell release];
      }
   }

   return self;
}

//_________________________________________________________________
- (void)dealloc
{
   [colorCells release];
   [patternCells release];

   [super dealloc];
}

//_________________________________________________________________
- (void)didReceiveMemoryWarning
{
   [super didReceiveMemoryWarning];
}

//_________________________________________________________________
- (void) setController : (ROOTObjectController *) p
{
   parentController = p;
}

//_________________________________________________________________
- (void) setObject : (TObject *)obj
{
   filledObject = dynamic_cast<TAttFill *>(obj);
   //Here I have to extract fill color and pattern from TAttFill.
}

//_________________________________________________________________
- (void) setNewColor : (NSInteger) cellIndex
{
   using namespace ROOT_IOSBrowser;

   if (filledObject && parentController) {
      if (cellIndex >= 0 && cellIndex < nROOTDefaultColors) {
         filledObject->SetFillColor(colorIndices[cellIndex]);
         [parentController objectWasModifiedByEditor];
      }
   }
}

//_________________________________________________________________
- (void) setNewPattern : (NSInteger) cellIndex
{
   if (filledObject && parentController) {
      if (cellIndex > 0 && cellIndex <= ROOT_iOS::GraphicUtils::kPredefinedFillPatterns) {
         filledObject->SetFillStyle(3000 + cellIndex);
      } else if (!cellIndex) {
         filledObject->SetFillStyle(1001);
      }

      [parentController objectWasModifiedByEditor];
   }
}

#pragma mark - View lifecycle

//_________________________________________________________________
- (void)viewDidLoad
{
   [super viewDidLoad];
}

//_________________________________________________________________
- (void)viewDidUnload
{
   [super viewDidUnload];
}

//_________________________________________________________________
- (BOOL)shouldAutorotateToInterfaceOrientation : (UIInterfaceOrientation)interfaceOrientation
{
   // Return YES for supported orientations
	return YES;
}

#pragma mark - color/pattern picker's dataSource.
//_________________________________________________________________
- (CGFloat)pickerView : (UIPickerView *)pickerView widthForComponent : (NSInteger)component
{
   return defaultCellW;
}

//_________________________________________________________________
- (CGFloat)pickerView : (UIPickerView *)pickerView rowHeightForComponent : (NSInteger)component
{
   return defaultCellH;
}

//_________________________________________________________________
- (NSInteger)pickerView : (UIPickerView *)pickerView numberOfRowsInComponent : (NSInteger)component
{
   if (pickerView == colorPicker)
      return [colorCells count];
   else if (pickerView == patternPicker)
      return [patternCells count];

   return 0;
}

//_________________________________________________________________
- (NSInteger)numberOfComponentsInPickerView:(UIPickerView *)pickerView
{
	return 1;
}

#pragma mark color/pattern picker's delegate.

// tell the picker which view to use for a given component and row, we have an array of views to show
//_________________________________________________________________
- (UIView *)pickerView : (UIPickerView *)pickerView viewForRow : (NSInteger)row forComponent : (NSInteger)component reusingView : (UIView *)view
{
   if (pickerView == colorPicker)
      return [colorCells objectAtIndex : row];
   else if (pickerView == patternPicker)
      return [patternCells objectAtIndex : row];

   return 0;
}

//_________________________________________________________________
- (void)pickerView : (UIPickerView *)thePickerView didSelectRow : (NSInteger)row inComponent : (NSInteger)component
{
   if (thePickerView == colorPicker)
      [self setNewColor : row];
   else if (thePickerView == patternPicker)
      [self setNewPattern : row];
}

@end
