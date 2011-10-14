#import "ROOTObjectController.h"
#import "HorizontalPickerView.h"
#import "LineStyleInspector.h"
#import "LineWidthPicker.h"
#import "LineStyleCell.h"
#import "ColorCell.h"
#import "Constants.h"

//C++ (ROOT) imports.
#import "TAttLine.h"
#import "TObject.h"

//TODO: remove line related constants to IOSLineStyles.h/*.cxx

static const CGRect cellFrame = CGRectMake(0.f, 0.f, 50.f, 50.f);



@implementation LineStyleInspector

@synthesize lineWidthView;

//____________________________________________________________________________________________________
- (id) initWithNibName : (NSString *)nibNameOrNil bundle : (NSBundle *)nibBundleOrNil
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
      
      lineStylePicker = [[HorizontalPickerView alloc] initWithFrame:CGRectMake(15.f, 20.f, 220.f, 70.f)];
      [lineStylePicker addItems : lineStyles];
      [self.view addSubview : lineStylePicker];
      [lineStylePicker release];
      
      
      lineColors = [[NSMutableArray alloc] init];
      for (unsigned i = 0; i < nROOTDefaultColors; ++i) {
         ColorCell *newCell = [[ColorCell alloc] initWithFrame : cellFrame];
         [newCell setRGB : predefinedFillColors[i]];
         [lineColors addObject : newCell];
         [newCell release];
      }

      lineColorPicker = [[HorizontalPickerView alloc] initWithFrame:CGRectMake(15.f, 95, 220.f, 70.f)];
      [lineColorPicker addItems : lineColors];
      [self.view addSubview : lineColorPicker];
      [lineColorPicker release];
      
   }

   return self;
}

//____________________________________________________________________________________________________
- (void) dealloc
{
   [lineStyles release];
   [lineColors release];
   
   self.lineWidthView = nil;

   [super dealloc];
}

//____________________________________________________________________________________________________
- (void) didReceiveMemoryWarning
{
   // Releases the view if it doesn't have a superview.
   [super didReceiveMemoryWarning];
   // Release any cached data, images, etc that aren't in use.
}

#pragma mark - View lifecycle

//____________________________________________________________________________________________________
- (void) viewDidLoad
{
   [super viewDidLoad];
   // Do any additional setup after loading the view from its nib.
}

//____________________________________________________________________________________________________
- (void) viewDidUnload
{
   [super viewDidUnload];
   // Release any retained subviews of the main view.
   // e.g. self.myOutlet = nil;
}

//____________________________________________________________________________________________________
- (BOOL) shouldAutorotateToInterfaceOrientation:(UIInterfaceOrientation)interfaceOrientation
{
   // Return YES for supported orientations
	return YES;
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
/*   
   const Style_t lineStyle = object->GetLineStyle();
   unsigned pickerRow = 0;
   if (lineStyle >= 1 && lineStyle <= 10)
      pickerRow = lineStyle - 1;

   [lineStylePicker selectRow : pickerRow inComponent : 0 animated : NO];*/
}

//____________________________________________________________________________________________________
- (IBAction) decLineWidth
{
//   NSLog(@"dec %@", lineWidthView);
   [lineWidthView decLineWidth];
}

//____________________________________________________________________________________________________
- (IBAction) incLineWidth
{
   NSLog(@"inc %@", lineWidthView);
   [lineWidthView incLineWidth];
}

@end
