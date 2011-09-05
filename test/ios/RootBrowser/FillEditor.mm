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
- (void) setROOTObjectController : (ROOTObjectController *) p
{
   parentController = p;
}

//_________________________________________________________________
- (void) setROOTObject : (TObject *)obj
{
   //ROOT's standard color pick has 16 colors,
   //I have 16 rows in a color picker.
   //Fill color is some integer index, not from [0 16),
   //but some hardcoded constant (as usually :( ) - 
   //see TGColorSelect or something like this.
   //I hold this indices in colorIndices array of constants,
   //since ROOT does not define them.
   //If the object color is one of 16 standard colors,
   //I find the correct row in a picker and rotate picker 
   //to this row. If not - it's on zero.
   using namespace ROOT_IOSBrowser;

   filledObject = dynamic_cast<TAttFill *>(obj);

   //Set the row in color picker, using fill color from object.
   const Color_t colorIndex = filledObject->GetFillColor();
   unsigned pickerRow = 0;
   for (unsigned i = 0; i < nROOTDefaultColors; ++i) {
      if (colorIndex == colorIndices[i]) {
         pickerRow = i;
         break;
      }
   }

   [fillPicker selectRow : pickerRow inComponent : 0 animated : NO];
   
   //Look for a fill pattern.
   namespace Fill = ROOT_iOS::GraphicUtils;
   
   const Style_t fillStyle = filledObject->GetFillStyle();
   if (fillStyle == Fill::solidFillStyle)//I'm sorry, again, hardcoded constant, ROOT does not define it :(.
      pickerRow = 0;
   else
      pickerRow = filledObject->GetFillStyle() % Fill::stippleBase;

   [fillPicker selectRow : pickerRow inComponent : 1 animated : NO];
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
   namespace Fill = ROOT_iOS::GraphicUtils;

   if (filledObject && parentController) {
      if (cellIndex > 0 && cellIndex <= Fill::kPredefinedFillPatterns) {
         filledObject->SetFillStyle(Fill::stippleBase + cellIndex);
      } else if (!cellIndex) {
         filledObject->SetFillStyle(Fill::solidFillStyle);
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
   if (!component)
      return [colorCells count];
   else
      return [patternCells count];
}

//_________________________________________________________________
- (NSInteger)numberOfComponentsInPickerView:(UIPickerView *)pickerView
{
	return 2;
}

#pragma mark color/pattern picker's delegate.

// tell the picker which view to use for a given component and row, we have an array of views to show
//_________________________________________________________________
- (UIView *)pickerView : (UIPickerView *)pickerView viewForRow : (NSInteger)row forComponent : (NSInteger)component reusingView : (UIView *)view
{
   if (!component)
      return [colorCells objectAtIndex : row];
   else
      return [patternCells objectAtIndex : row];
}

//_________________________________________________________________
- (void)pickerView : (UIPickerView *)thePickerView didSelectRow : (NSInteger)row inComponent : (NSInteger)component
{
   if (!component)
      [self setNewColor : row];
   else
      [self setNewPattern : row];
}

@end
