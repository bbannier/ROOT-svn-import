#import "H1BinsInspector.h"
#import "RangeSlider.h"

@implementation H1BinsInspector

@synthesize minLabel;
@synthesize maxLabel;

//____________________________________________________________________________________________________
- (id) initWithNibName : (NSString *)nibNameOrNil bundle : (NSBundle *)nibBundleOrNil
{
   self = [super initWithNibName : nibNameOrNil bundle : nibBundleOrNil];
   if (self) {
      [self view];
      
      //
           //
   }

   return self;
}

//____________________________________________________________________________________________________
- (void) dealloc
{
   self.minLabel = nil;
   self.maxLabel = nil;
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

   axisRangeSlider = [[RangeSlider alloc] initWithFrame : CGRectMake(0.f, 210.f, 250.f, 60.f) min : 0.f max : 10.f selectedMin : 1.f selectedMax : 9.f];
   [self.view addSubview : axisRangeSlider];
   [axisRangeSlider release];
   
   [axisRangeSlider addTarget:self action:@selector(axisRangeChanged) forControlEvents : UIControlEventValueChanged];
}

//____________________________________________________________________________________________________
- (void) viewDidUnload
{
   [super viewDidUnload];
   // Release any retained subviews of the main view.
   // e.g. self.myOutlet = nil;
}

//____________________________________________________________________________________________________
- (BOOL) shouldAutorotateToInterfaceOrientation : (UIInterfaceOrientation)interfaceOrientation
{
   // Return YES for supported orientations
	return YES;
}

//____________________________________________________________________________________________________
-(void) axisRangeChanged
{
   minLabel.center = CGPointMake([axisRangeSlider minThumbX], minLabel.center.y);
   minLabel.text = [NSString stringWithFormat:@"%.3g", axisRangeSlider.selectedMinimumValue];
   maxLabel.center = CGPointMake([axisRangeSlider maxThumbX], maxLabel.center.y);
   maxLabel.text = [NSString stringWithFormat:@"%.3g", axisRangeSlider.selectedMaximumValue];
}

@end
