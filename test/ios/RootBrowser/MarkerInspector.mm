#import "MarkerInspector.h"

#import "TAttMarker.h"
#import "TObject.h"

@implementation MarkerInspector

@synthesize markerPicker;

const CGFloat defaultCellW = 80.f;
const CGFloat defaultCellH = 44.f;

//____________________________________________________________________________________________________
- (id) initWithNibName : (NSString *)nibNameOrNil bundle : (NSBundle *)nibBundleOrNil
{
   self = [super initWithNibName : nibNameOrNil bundle : nibBundleOrNil];
   
   if (self) {
      [self view];
   }

   return self;
}

//____________________________________________________________________________________________________
- (void) dealloc 
{
   self.markerPicker = nil;
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
   return 0;
}

//____________________________________________________________________________________________________
- (NSInteger)numberOfComponentsInPickerView:(UIPickerView *)pickerView
{
	return 2;
}

//____________________________________________________________________________________________________
- (UIView *)pickerView : (UIPickerView *)pickerView viewForRow : (NSInteger)row forComponent : (NSInteger)component reusingView : (UIView *)view
{
   return nil;
}

//____________________________________________________________________________________________________
- (void)pickerView : (UIPickerView *)thePickerView didSelectRow : (NSInteger)row inComponent : (NSInteger)component
{
}

#pragma mark ObjectInspectorComponent protocol.

//____________________________________________________________________________________________________
- (void) setROOTObject : (TObject *)o
{
   object = dynamic_cast<TAttMarker*>(o);
}

//____________________________________________________________________________________________________
- (void) setROOTObjectController : (ROOTObjectController *)c
{
   controller = c;
}

//____________________________________________________________________________________________________
- (NSString *) getComponentName
{
   return @"Marker attributes";
}

@end
