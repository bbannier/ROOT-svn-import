#import "H1BinsInspector.h"

@implementation H1BinsInspector

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
- (BOOL) shouldAutorotateToInterfaceOrientation : (UIInterfaceOrientation)interfaceOrientation
{
   // Return YES for supported orientations
	return YES;
}

@end
