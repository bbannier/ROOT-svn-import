//
//  RootFileController.m
//  root_browser
//
//  Created by Timur Pocheptsov on 8/19/11.
//  Copyright 2011 __MyCompanyName__. All rights reserved.
//

#import "FileContentController.h"
#import "RootFileController.h"
#import "FileShortcut.h"


@implementation RootFileController

- (id)initWithNibName : (NSString *)nibNameOrNil bundle : (NSBundle *)nibBundleOrNil
{
   self = [super initWithNibName : nibNameOrNil bundle : nibBundleOrNil];
   if (self) {
      self.navigationItem.title = @"ROOT files";
      UIBarButtonItem *backButton = [[UIBarButtonItem alloc] initWithTitle : @"Back to ROOT files" style:UIBarButtonItemStylePlain target : nil action : nil];
      self.navigationItem.backBarButtonItem = backButton;
      [backButton release];
   }
   return self;
}

- (void)dealloc
{
   [contentController release];
   [super dealloc];
}

- (void)didReceiveMemoryWarning
{
    // Releases the view if it doesn't have a superview.
    [super didReceiveMemoryWarning];
    
    // Release any cached data, images, etc that aren't in use.
}

#pragma mark - View lifecycle

- (void)viewDidLoad
{
   UIScrollView *scroll = (UIScrollView *)self.view;
   scroll.contentSize = CGSizeMake(2000.f, 2000.f);
   [super viewDidLoad];
   // Do any additional setup after loading the view from its nib.
}

- (void)viewDidUnload
{
    [super viewDidUnload];
    // Release any retained subviews of the main view.
    // e.g. self.myOutlet = nil;
}

- (BOOL)shouldAutorotateToInterfaceOrientation:(UIInterfaceOrientation)interfaceOrientation
{
    // Return YES for supported orientations
	return YES;
}

#pragma mark View management.

- (void) addFiles //addFile : (ROOT_iOS::FileUtils::FileContainer.
{
   if (!fileContainers)
      fileContainers = [[NSMutableArray alloc] init];
   
   const CGFloat addSpace = 50.f;
   const unsigned nFiles = 2;
   
   const CGRect scrollFrame = self.view.frame;
   CGRect shortcutFrame = CGRectMake(0.f, 0.f, [FileShortcut fileIconWidth], [FileShortcut fileIconHeight]);
   const unsigned nPicksInRow = scrollFrame.size.width / (shortcutFrame.size.width + addSpace);
   
   for (unsigned i = 0; i < nFiles; ++i) {
      const unsigned row = i / nPicksInRow;
      const unsigned col = i % nPicksInRow;
      
      shortcutFrame.origin = CGPointMake(addSpace + col * ([FileShortcut fileIconWidth] + addSpace), addSpace + row * ([FileShortcut fileIconHeight] + addSpace));
      NSString *fileName = [NSString stringWithFormat:@"hists_%u_%u.root", row, col];
      FileShortcut *newIcon = [[FileShortcut alloc] initWithFrame : shortcutFrame controller : self fileName : fileName contents : 10];
      [self.view addSubview : newIcon];
      [newIcon release];
   }
}

- (void) fileWasSelected : (FileShortcut*) shortcut
{
   if (!contentController)
      contentController = [[FileContentController alloc] initWithNibName : @"FileContentController" bundle : nil];
   [contentController activateForFile : shortcut.fileName];
   [self.navigationController pushViewController : contentController animated : YES];
}

@end
