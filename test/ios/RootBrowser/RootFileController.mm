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

//____________________________________________________________________________________________________
- (id)initWithNibName : (NSString *)nibNameOrNil bundle : (NSBundle *)nibBundleOrNil
{
   self = [super initWithNibName : nibNameOrNil bundle : nibBundleOrNil];
   
   [self view]; //Nib is lazy loading, many thanks to brilliant apple's developers.
   
   if (self) {
      self.navigationItem.title = @"ROOT files";
      UIBarButtonItem *backButton = [[UIBarButtonItem alloc] initWithTitle : @"Back to ROOT files" style:UIBarButtonItemStylePlain target : nil action : nil];
      self.navigationItem.backBarButtonItem = backButton;
      UIBarButtonItem *leftButton = [[UIBarButtonItem alloc] initWithTitle : @"Open file" style:UIBarButtonItemStylePlain target : self action : @selector(openFile)];
      self.navigationItem.leftBarButtonItem = leftButton;
      [backButton release];
   }

   return self;
}

//____________________________________________________________________________________________________
- (void)dealloc
{
//   self.scrollView = nil;
   [contentController release];
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
   scrollView.bounces = NO;
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

//____________________________________________________________________________________________________
- (void) placeFileShortcuts
{
   [ShorcutUtil placeShortcuts : fileContainers inScrollView : scrollView withSize : CGSizeMake([FileShortcut iconWidth], [FileShortcut iconHeight]) andSpace : 25.f];
}

//____________________________________________________________________________________________________
- (void)didRotateFromInterfaceOrientation:(UIInterfaceOrientation)fromInterfaceOrientation
{
   [self placeFileShortcuts];
}

#pragma mark View management.

//____________________________________________________________________________________________________
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

//____________________________________________________________________________________________________
- (void) fileWasSelected : (FileShortcut*) shortcut
{
   if (!contentController)
      contentController = [[FileContentController alloc] initWithNibName : @"FileContentController" bundle : nil];
      
   [contentController activateForFile : [shortcut getFileContainer]];
   [self.navigationController pushViewController : contentController animated : YES];
}

//____________________________________________________________________________________________________
- (void) openFile
{
   fileOpenView.hidden = !fileOpenView.hidden;
}

@end
