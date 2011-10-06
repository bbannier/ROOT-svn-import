#import "ROOTObjectController.h"
#import "MarkerInspector.h"
#import "MarkerStyleCell.h"
#import "ColorCell.h"
#import "Constants.h"

#import "TAttMarker.h"
#import "TObject.h"

@implementation MarkerInspector

@synthesize markerPicker;
@synthesize plusBtn;
@synthesize minusBtn;
@synthesize sizeLabel;

const CGFloat defaultCellW = 80.f;
const CGFloat defaultCellH = 44.f;

const CGFloat maxMarkerSize = 5.f;
const CGFloat sizeStep = 0.1f;

EMarkerStyle markerStyles[] = {kDot, kPlus, kStar, kCircle, kMultiply,
                          kFullDotSmall, kFullDotMedium, kFullDotLarge,
                          kFullCircle, kFullSquare, kFullTriangleUp,
                          kFullTriangleDown, kOpenCircle, kOpenSquare,
                          kOpenTriangleUp, kOpenDiamond, kOpenCross,
                          kFullStar, kOpenStar, kOpenTriangleDown,
                          kFullDiamond, kFullCross};

const unsigned nMarkers = sizeof markerStyles / sizeof markerStyles[0];

//____________________________________________________________________________________________________
static BOOL canScaleMarker(Style_t style)
{
   if (style == kDot || style == kFullDotSmall || style == kFullDotMedium)
      return NO;
   return YES;
}

//____________________________________________________________________________________________________
- (id) initWithNibName : (NSString *)nibNameOrNil bundle : (NSBundle *)nibBundleOrNil
{
   using namespace ROOT_IOSBrowser;

   self = [super initWithNibName : nibNameOrNil bundle : nibBundleOrNil];
   
   if (self) {
      [self view];
      
      const CGRect cellFrame = CGRectMake(0.f, 0.f, defaultCellW, defaultCellH);
      
      styleCells = [[NSMutableArray alloc] init];//]WithCapacity : nMarkers];
      for (unsigned i = 0; i < nMarkers; ++i) {
         MarkerStyleCell *newCell = [[MarkerStyleCell alloc] initWithFrame : cellFrame andMarkerStyle : markerStyles[i]];
         [styleCells addObject : newCell];
         [newCell release];
      }
      
      colorCells = [[NSMutableArray alloc] init];
      for (unsigned i = 0; i < nROOTDefaultColors; ++i) {
         ColorCell *newCell = [[ColorCell alloc] initWithFrame : cellFrame];
         [newCell setRGB : predefinedFillColors[i]];
         [colorCells addObject : newCell];
         [newCell release];
      }
   }

   return self;
}

//____________________________________________________________________________________________________
- (void) dealloc 
{
   self.markerPicker = nil;
   self.plusBtn = nil;
   self.minusBtn = nil;
   self.sizeLabel = nil;
   
   [styleCells release];
   [colorCells release];

   [super dealloc];
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

#pragma mark - Pickerview delegate/data source.

//____________________________________________________________________________________________________
- (CGFloat)pickerView : (UIPickerView *)pickerView widthForComponent : (NSInteger)component
{
   return defaultCellW;
}

//____________________________________________________________________________________________________
- (CGFloat)pickerView : (UIPickerView *)pickerView rowHeightForComponent : (NSInteger)component
{
   return defaultCellH;
}

//____________________________________________________________________________________________________
- (NSInteger)pickerView : (UIPickerView *)pickerView numberOfRowsInComponent : (NSInteger)component
{
   if (!component)
      return [styleCells count];

   return [colorCells count];
}

//____________________________________________________________________________________________________
- (NSInteger)numberOfComponentsInPickerView:(UIPickerView *)pickerView
{
	return 2;
}

//____________________________________________________________________________________________________
- (UIView *)pickerView : (UIPickerView *)pickerView viewForRow : (NSInteger)row forComponent : (NSInteger)component reusingView : (UIView *)view
{
   if (!component)
      return [styleCells objectAtIndex : row];
   return [colorCells objectAtIndex : row];
}

//____________________________________________________________________________________________________
- (void)pickerView : (UIPickerView *)thePickerView didSelectRow : (NSInteger)row inComponent : (NSInteger)component
{
   if (!component) {
      if (row >= 0 && row < nMarkers) {
         EMarkerStyle style = markerStyles[row];
         if (canScaleMarker(style)) {
            plusBtn.enabled = YES;
            minusBtn.enabled = YES;
            sizeLabel.text = [NSString stringWithFormat : @"%.2g", object->GetMarkerSize()];
         } else {
            plusBtn.enabled = NO;
            minusBtn.enabled = NO;
            sizeLabel.text = @"1";
         }
         
         object->SetMarkerStyle(style);
      }
   } else {
      const unsigned colorIndex = ROOT_IOSBrowser::colorIndices[row];
      object->SetMarkerColor(colorIndex);
   }

   [controller objectWasModifiedUpdateSelection : NO];
}

#pragma mark ObjectInspectorComponent protocol.

//____________________________________________________________________________________________________
- (void) setROOTObjectController : (ROOTObjectController *)c
{
   controller = c;
}

//____________________________________________________________________________________________________
- (void) setROOTObject : (TObject *)o
{
   using namespace ROOT_IOSBrowser;

   object = dynamic_cast<TAttMarker *>(o);

   unsigned row = 0;
   const EMarkerStyle style = EMarkerStyle(object->GetMarkerStyle());//Mess with all these styles and EMarkerStyles.
   for (unsigned i = 0; i < nMarkers; ++i) {
      if (style == markerStyles[i]) {
         row = i;
         break;
      }
   }
   
   [markerPicker selectRow : NSInteger(row) inComponent : 0 animated : NO];

   //Extract marker color.
   //The same predefined 16 colors as with fill color.
   row = 1;//?
   const Color_t colorIndex = object->GetMarkerColor();
   
   for (unsigned i = 0; i < nROOTDefaultColors; ++i) {
      if (colorIndex == colorIndices[i]) {
         row = i;
         break;
      }
   }
   
   [markerPicker selectRow : NSInteger(row) inComponent : 1 animated : NO];

   if (!canScaleMarker(object->GetMarkerStyle())) {
      plusBtn.enabled = NO;
      minusBtn.enabled = NO;
      sizeLabel.text = @"1";
   } else {
      plusBtn.enabled = YES;
      minusBtn.enabled = YES;
      sizeLabel.text = [NSString stringWithFormat : @"%.2g", object->GetMarkerSize()];
   }
}

//____________________________________________________________________________________________________
- (NSString *) getComponentName
{
   return @"Marker attributes";
}

//____________________________________________________________________________________________________
- (IBAction) plusPressed
{

   if (object->GetMarkerSize() + sizeStep > maxMarkerSize)
      return;

   object->SetMarkerSize(object->GetMarkerSize() + sizeStep);
   sizeLabel.text = [NSString stringWithFormat : @"%.2g", object->GetMarkerSize()];   
   [controller objectWasModifiedUpdateSelection : NO];
}

//____________________________________________________________________________________________________
- (IBAction) minusPressed
{
   if (object->GetMarkerSize() - sizeStep < 1.)
      return;
   
   object->SetMarkerSize(object->GetMarkerSize() - sizeStep);
   sizeLabel.text = [NSString stringWithFormat : @"%.2g", object->GetMarkerSize()];   
   [controller objectWasModifiedUpdateSelection : NO];
}

@end
