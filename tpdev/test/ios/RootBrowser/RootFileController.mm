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
#import "Shortcuts.h"

@implementation RootFileController

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
/*   const CGRect scrollFrame = scrollView.frame;
   const CGFloat addSpace = 25.f;
   const CGFloat shortcutWidth = [FileShortcut iconWidth];
   const CGFloat shortcutHeight = [FileShortcut iconHeight];
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
   
  */ 
//   PlaceShortcutsInAScrollView(fileContainers, scrollView, [FileShortcut iconWidth], [FileShortcut iconHeight], 25.f);
   [ShorcutUtil placeShortcuts : fileContainers inScrollView : scrollView withSize : CGSizeMake([FileShortcut iconWidth], [FileShortcut iconHeight]) andSpace : 25.f];
}

- (void)didRotateFromInterfaceOrientation:(UIInterfaceOrientation)fromInterfaceOrientation
{
   [self placeFileShortcuts];
}

#pragma mark View management.

//- (void) addFileShortcut : (ROOT_iOS::FileUtils::FileContainer*) file
- (void) addFileShortcut : (NSString *) fileName
{
   if (!fileContainers)
      fileContainers = [[NSMutableArray alloc] init];
   
   const CGRect shortcutFrame = CGRectMake(0.f, 0.f, [FileShortcut iconWidth], [FileShortcut iconHeight]);
   FileShortcut *newShortcut = [[FileShortcut alloc] initWithFrame:shortcutFrame controller:self filePath : fileName];

   if (![newShortcut getFileContainer]) {
      UIAlertView *alert = [[UIAlertView alloc] initWithTitle : @"File Open Error:"
                                                message : [NSString stringWithFormat:@"Could not open %@", fileName]
                                                delegate : nil
                                                cancelButtonTitle : @"Close"
                                                otherButtonTitles : nil];
      [alert show];
      [alert release];
      [newShortcut release];

      return;
   }

   [fileContainers addObject : newShortcut];
   [scrollView addSubview : newShortcut];
   [newShortcut release];
   
   [self placeFileShortcuts];
}

- (void) fileWasSelected : (FileShortcut*) shortcut
{
   if (!contentController)
      contentController = [[FileContentController alloc] initWithNibName : @"FileContentController" bundle : nil];
      
   [contentController activateForFile : [shortcut getFileContainer]];
   [self.navigationController pushViewController : contentController animated : YES];
}

@end
