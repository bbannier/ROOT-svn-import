//
//  RootFileController.m
//  root_browser
//
//  Created by Timur Pocheptsov on 8/19/11.
//  Copyright 2011 __MyCompanyName__. All rights reserved.
//

#import <Foundation/NSEnumerator.h>

#import "FileContentController.h"
#import "RootFileController.h"
#import "FileShortcut.h"


@implementation RootFileController

@synthesize toolBarView;
@synthesize scrollView;

- (id)initWithNibName : (NSString *)nibNameOrNil bundle : (NSBundle *)nibBundleOrNil
{
   self = [super initWithNibName : nibNameOrNil bundle : nibBundleOrNil];
   
   [self view]; //Nib is lazy loading, many thanks to brilliant apple's developers.
   
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
//   self.scrollView = nil;
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
   [super viewDidLoad];
   scrollView.bounces = NO;
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

- (void) placeFileShortcuts
{
   const CGRect scrollFrame = scrollView.frame;
   const CGFloat addSpace = 25.f;
   const CGFloat shortcutWidth = [FileShortcut fileIconWidth];
   const CGFloat shortcutHeight = [FileShortcut fileIconHeight];
   const unsigned nPicksInRow = scrollFrame.size.width / (shortcutWidth + addSpace);
   const CGFloat addXY = (scrollFrame.size.width - (shortcutWidth + addSpace) * nPicksInRow) / 2;
   
   NSEnumerator *enumerator = [fileContainers objectEnumerator];
   UIView *v = [enumerator nextObject];
   for (unsigned n = 0; v; v = [enumerator nextObject], ++n) {
      const unsigned col = n % nPicksInRow;
      const unsigned row = n / nPicksInRow;
      
      const CGFloat x = addXY + addSpace / 2 + col * (shortcutWidth + addSpace);
      const CGFloat y = row * shortcutHeight + addXY;

      CGRect frame = v.frame;
      frame.origin = CGPointMake(x, y);
      v.frame = frame;
   }
   
   scrollView.contentSize = CGSizeMake(scrollFrame.size.width, addXY + ([fileContainers count] + nPicksInRow - 1) / nPicksInRow * shortcutHeight);
}

- (void)didRotateFromInterfaceOrientation:(UIInterfaceOrientation)fromInterfaceOrientation
{
   [self placeFileShortcuts];
}

#pragma mark View management.

//- (void) addFileShortcut : (ROOT_iOS::FileUtils::FileContainer*) file
- (void) addFileShortcuts
{
   if (!fileContainers)
      fileContainers = [[NSMutableArray alloc] init];
   
   const unsigned nFiles = 20;
   const CGRect shortcutFrame = CGRectMake(0.f, 0.f, [FileShortcut fileIconWidth], [FileShortcut fileIconHeight]);
   
   for (unsigned i = 0; i < nFiles; ++i) {
      NSString *fileName = [NSString stringWithFormat:@"hists_%u.root", i];
      FileShortcut *newShortcut = [[FileShortcut alloc] initWithFrame : shortcutFrame controller : self fileName : fileName contents : 10];
      [self.scrollView addSubview : newShortcut];
      [fileContainers addObject : newShortcut];
      [newShortcut release];
   }
   
   [self placeFileShortcuts];
}

- (void) fileWasSelected : (FileShortcut*) shortcut
{
   if (!contentController)
      contentController = [[FileContentController alloc] initWithNibName : @"FileContentController" bundle : nil];
      
   [contentController activateForFile : shortcut.fileName];
   [self.navigationController pushViewController : contentController animated : YES];
}

@end
