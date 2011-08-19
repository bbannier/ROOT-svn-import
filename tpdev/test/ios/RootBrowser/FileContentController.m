//
//  FileContentController.m
//  root_browser
//
//  Created by Timur Pocheptsov on 8/19/11.
//  Copyright 2011 __MyCompanyName__. All rights reserved.
//

#import "FileContentController.h"
#import "ROOTObjectController.h"
#import "SlideshowController.h"
#import "ObjectShortcut.h"


@implementation FileContentController

@synthesize fileName;

- (id)initWithNibName : (NSString *)nibNameOrNil bundle : (NSBundle *)nibBundleOrNil
{
   self = [super initWithNibName:nibNameOrNil bundle:nibBundleOrNil];
   if (self) {
      [fileName retain];
      self.navigationItem.rightBarButtonItem = [[UIBarButtonItem alloc] initWithTitle:@"Slide show" style:UIBarButtonItemStyleBordered target:self action:@selector(startSlideshow)];
      
      CGRect frame = [ObjectShortcut defaultRect];
      frame.origin = CGPointMake(50.f, 50.f);
      ObjectShortcut *sh = [[ObjectShortcut alloc] initWithFrame : frame controller : self objectName : @"sample_histo"];
      [self.view addSubview : sh];
      [sh release];
      
      UIScrollView *scroll = (UIScrollView *)self.view;
      scroll.contentSize = CGSizeMake(1000.f, 1000.f);
   }

   return self;
}

- (void)dealloc
{
   self.fileName = nil;
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

- (void) activateForFile : (NSString *)name
{
   self.navigationItem.title = [NSString stringWithFormat :@"Contents of %@", name];
}

- (void) startSlideshow
{
   if (!slideshowController)
      slideshowController = [[SlideshowController alloc] initWithNibName : @"SlideshowController" bundle : nil];
      
   [self.navigationController pushViewController : slideshowController animated : YES];
}

- (void) selectFromFile : (ObjectShortcut *) obj
{
   if (!objectController)
      objectController = [[ROOTObjectController alloc] initWithNibName:@"ROOTObjectController" bundle : nil];
   
   objectController.navigationItem.title = obj.objectName;
   [self.navigationController pushViewController : objectController animated : YES];
}

@end
