#import "PatternCell.h"
#import "FillEditor.h"
#import "ColorCell.h"

//C++ (ROOT) imports:
#import "IOSFillPatterns.h"
#import "TAttFill.h"


//Fill colors, visible in a ROOT's default color-picker (in editors).
const unsigned nROOTDefaultColors = 16;
const CGFloat defaultCellW = 80.f;
const CGFloat defaultCellH = 44.f;

static const double predefinedFillColors[nROOTDefaultColors][3] = 
{
{1., 1., 1.},
{0., 0., 0.},
{251 / 255., 0., 24 / 255.},
{40 / 255., 253 / 255., 44 / 255.},
{31 / 255., 29 / 255., 251 / 255.},
{253 / 255., 254 / 255., 52 / 255.},
{253 / 255., 29 / 255., 252 / 255.},
{53 / 255., 1., 254 / 255.},
{94 / 255., 211 / 255., 90 / 255.},
{92 / 255., 87 / 255., 214 / 255.},
{135 / 255., 194 / 255., 164 / 255.},
{127 / 255., 154 / 255., 207 / 255.},
{211 / 255., 206 / 255., 138 / 255.},
{220 / 255., 185 / 255., 138 / 255.},
{209 / 255., 89 / 255., 86 / 255.},
{147 / 255., 29 / 255., 251 / 255.}
};

//Color indices in a standard ROOT's color selection control.
static const unsigned colorIndices[16] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 30, 38, 41, 42, 50, 51};

@implementation FillEditor

//_________________________________________________________________
- (id)initWithNibName : (NSString *)nibNameOrNil bundle : (NSBundle *)nibBundleOrNil
{
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
- (void) setParentController : (id) p
{
   parentController = p;
}

//_________________________________________________________________
- (void) setObject : (TAttFill *)obj
{
   filledObject = obj;
   //Here I have to extract fill color and pattern from TAttFill.
}

//_________________________________________________________________
- (void) setNewColor : (NSInteger) cellIndex
{
   if (filledObject && parentController) {
      if (cellIndex >= 0 && cellIndex < nROOTDefaultColors) {
         filledObject->SetFillColor(colorIndices[cellIndex]);
         [parentController propertyUpdated];
      }
   }
}

//_________________________________________________________________
- (void) setNewPattern : (NSInteger) cellIndex
{
   if (filledObject && parentController) {
      if (cellIndex > 0 && cellIndex <= ROOT_iOS::GraphicUtils::kPredefinedFillPatterns) {
         filledObject->SetFillStyle(3000 + cellIndex);
         [parentController propertyUpdated];
      } else if (!cellIndex) {
         filledObject->SetFillStyle(1001);
         [parentController propertyUpdated];
      }
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
