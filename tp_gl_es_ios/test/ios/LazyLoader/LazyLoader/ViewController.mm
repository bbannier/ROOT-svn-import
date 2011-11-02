#import "ViewController.h"

#import "FileUtils.h"

@implementation ViewController

- (void)didReceiveMemoryWarning
{
    [super didReceiveMemoryWarning];
    // Release any cached data, images, etc that aren't in use.
}

#pragma mark - View lifecycle

- (void)viewDidLoad
{
    [super viewDidLoad];
	// Do any additional setup after loading the view, typically from a nib.
   using namespace ROOT::iOS::Browser;
   
   NSString *filePath = [[NSBundle mainBundle] pathForResource : @"mo" ofType : @"root"];
   if (filePath) {
      NSLog(@"Test file utils:");
      FileContainer *fileContainer = CreateROOTFileContainer([filePath cStringUsingEncoding:[NSString defaultCStringEncoding]]);
      if (fileContainer)
         NSLog(@"Object keys read from file %lu", fileContainer->GetNumberOfObjects());
      DeleteROOTFileContainer(fileContainer);
      NSLog(@"Test finished ok");
   }
}

- (void)viewDidUnload
{
    [super viewDidUnload];
    // Release any retained subviews of the main view.
    // e.g. self.myOutlet = nil;
}

- (void)viewWillAppear:(BOOL)animated
{
    [super viewWillAppear:animated];
}

- (void)viewDidAppear:(BOOL)animated
{
    [super viewDidAppear:animated];
}

- (void)viewWillDisappear:(BOOL)animated
{
	[super viewWillDisappear:animated];
}

- (void)viewDidDisappear:(BOOL)animated
{
	[super viewDidDisappear:animated];
}

- (BOOL)shouldAutorotateToInterfaceOrientation:(UIInterfaceOrientation)interfaceOrientation
{
    // Return YES for supported orientations
   return YES;
}

@end
