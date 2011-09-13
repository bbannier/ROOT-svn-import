#import "ROOTObjectController.h"
#import "AxisColorInspector.h"
#import "Constants.h"
#import "ColorCell.h"

//C++ (ROOT) imports:
#import "TAttAxis.h"
#import "TObject.h"

//TODO: check, if in Obj-C++ constants have internal linkage.
static const CGFloat defaultCellW = 180.f;
static const CGFloat defaultCellH = 44.f;

@implementation AxisColorInspector

//_________________________________________________________________
- (id)initWithNibName : (NSString *)nibNameOrNil bundle : (NSBundle *)nibBundleOrNil mode : (ROOT_IOSObjectInspector::AxisColorInspectorMode)m
{
   using namespace ROOT_IOSBrowser;
   using namespace ROOT_IOSObjectInspector;

   self = [super initWithNibName : nibNameOrNil bundle : nibBundleOrNil];

   [self view];

   if (self) {
      const CGRect cellRect = CGRectMake(0.f, 0.f, defaultCellW, defaultCellH);
      mode = m;
   
      colors = [[NSMutableArray alloc] init];

      for (unsigned i = 0; i < nROOTDefaultColors; ++i) {
         ColorCell * newCell = [[ColorCell alloc] initWithFrame : cellRect];
         [newCell setRGB : predefinedFillColors[i]];
         [colors addObject : newCell];
         [newCell release];
      }
      
      if (mode == acimAxisColor)
         titleLabel.text = @"Axis color:";
      else if (mode == acimTitleColor)
         titleLabel.text = @"Title color:";
      else if (mode == acimLabelColor)
         titleLabel.text = @"Label color:";
   }

   return self;
}

//_________________________________________________________________
- (void)dealloc
{
   [colors release];

   [super dealloc];
}

//_________________________________________________________________
- (void)didReceiveMemoryWarning
{
   [super didReceiveMemoryWarning];
}

//_________________________________________________________________
- (void) setROOTObjectController : (ROOTObjectController *)c
{
   controller = c;
}

//_________________________________________________________________
- (void) setROOTObject : (TObject *)obj
{
   using namespace ROOT_IOSBrowser;
   using namespace ROOT_IOSObjectInspector;
   //I do not check the result of dynamic_cast here. This is done at upper level.
   object = dynamic_cast<TAttAxis *>(obj);

   //Set the row in color picker, using fill color from object.
   Color_t colorIndex = 0;
   if (mode == acimAxisColor)
      colorIndex = object->GetAxisColor();
   else if (mode == acimTitleColor)
      colorIndex = object->GetTitleColor();
   else if (mode == acimLabelColor)
      colorIndex = object->GetLabelColor();
   
   unsigned pickerRow = 0;
   for (unsigned i = 0; i < nROOTDefaultColors; ++i) {
      if (colorIndex == colorIndices[i]) {
         pickerRow = i;
         break;
      }
   }

   [colorPicker selectRow : pickerRow inComponent : 0 animated : NO];
}

//_________________________________________________________________
- (void) setNewColor : (NSInteger) row
{
   using namespace ROOT_IOSBrowser;
   using namespace ROOT_IOSObjectInspector;

   if (object && controller) {
      if (mode == acimAxisColor)
         object->SetAxisColor(colorIndices[row]);
      else if (mode == acimTitleColor)
         object->SetTitleColor(colorIndices[row]);
      else if (mode == acimLabelColor)
         object->SetLabelColor(colorIndices[row]);
      [controller objectWasModifiedUpdateSelection : NO];
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
   return [colors count];
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
   return [colors objectAtIndex : row];
}

//_________________________________________________________________
- (void)pickerView : (UIPickerView *)thePickerView didSelectRow : (NSInteger)row inComponent : (NSInteger)component
{
   [self setNewColor : row];
}

//_________________________________________________________________
- (void) back
{
   [self.navigationController popViewControllerAnimated : YES];
}

@end
