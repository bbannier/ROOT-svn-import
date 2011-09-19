#import "ROOTObjectController.h"
#import "LineStyleInspector.h"
#import "LineStyleCell.h"
#import "Constants.h"

//C++ (ROOT) imports.
#import "TAttLine.h"
#import "TObject.h"

//TODO: remove line related constants to IOSLineStyles.h/*.cxx

static const CGRect cellFrame = CGRectMake(0.f, 0.f, 180.f, 44.f);

@implementation LineStyleInspector

@synthesize lineStylePicker;

//____________________________________________________________________________________________________
- (id)initWithNibName : (NSString *) nibNameOrNil bundle : (NSBundle *)nibBundleOrNil
{
   using namespace ROOT_IOSBrowser;

   self = [super initWithNibName : nibNameOrNil bundle : nibBundleOrNil];

   [self view];//To fight lazy nib loading.

   if (self) {
      //Array with cells for "Line style" picker.
      lineStyles = [[NSMutableArray alloc] init];
      for (unsigned i = 0; i < 10; ++i) {
         LineStyleCell *newCell = [[LineStyleCell alloc] initWithFrame : cellFrame lineStyle : i + 1];
         [lineStyles addObject : newCell];
         [newCell release];
      }
   }

   return self;
}

//____________________________________________________________________________________________________
- (void)dealloc
{
   [lineStyles release];
   self.lineStylePicker = nil;

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

#pragma mark - Color/Width/Style picker's dataSource.
//____________________________________________________________________________________________________
- (CGFloat)pickerView : (UIPickerView *)pickerView widthForComponent : (NSInteger)component
{
   return cellFrame.size.width;
}

//____________________________________________________________________________________________________
- (CGFloat)pickerView : (UIPickerView *)pickerView rowHeightForComponent : (NSInteger)component
{
   return cellFrame.size.height;
}

//____________________________________________________________________________________________________
- (NSInteger)pickerView : (UIPickerView *)pickerView numberOfRowsInComponent : (NSInteger)component
{
   return [lineStyles count];

   return 0;
}

//____________________________________________________________________________________________________
- (NSInteger)numberOfComponentsInPickerView : (UIPickerView *)pickerView
{
	return 1;
}

#pragma mark color/pattern picker's delegate.

//____________________________________________________________________________________________________
- (UIView *)pickerView : (UIPickerView *)pickerView viewForRow : (NSInteger)row forComponent : (NSInteger)component reusingView : (UIView *)view
{
   return [lineStyles objectAtIndex : row];
}

//____________________________________________________________________________________________________
- (void)pickerView : (UIPickerView *)thePickerView didSelectRow : (NSInteger)row inComponent : (NSInteger)component
{
   object->SetLineStyle(row + 1);
   
   [controller objectWasModifiedUpdateSelection : NO];
}

//____________________________________________________________________________________________________
- (void) setROOTObjectController : (ROOTObjectController *) c
{
   controller = c;
}

//____________________________________________________________________________________________________
- (void) setROOTObject : (TObject *) obj
{
   //As usually, ROOT is a mess. Line style can be 0, can be 1 - this is solid line.
   //And there are 10 line styles including solid line. But actually 11. I hate this mess.

   //I do not check the result of dynamic_cast here. This is done at upper level.
   object = dynamic_cast<TAttLine *>(obj);
   
   const Style_t lineStyle = object->GetLineStyle();
   unsigned pickerRow = 0;
   if (lineStyle >= 1 && lineStyle <= 10)
      pickerRow = lineStyle - 1;

   [lineStylePicker selectRow : pickerRow inComponent : 0 animated : NO];
}

//____________________________________________________________________________________________________
- (void) back
{
   [self.navigationController popViewControllerAnimated : YES];
}

@end
