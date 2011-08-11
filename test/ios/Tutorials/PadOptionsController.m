//
//  PadOptionsController.m
//  Tutorials
//
//  Created by Timur Pocheptsov on 8/11/11.
//  Copyright 2011 __MyCompanyName__. All rights reserved.
//

#import "PadOptionsController.h"


@implementation PadOptionsController

@synthesize tickX = tickX_;
@synthesize tickY = tickY_;
@synthesize gridX = gridX_;
@synthesize gridY = gridY_;
@synthesize logX = logX_;
@synthesize logY = logY_;
@synthesize logZ = logZ_;
@synthesize colorPicker = colorPicker_;
@synthesize patternPicker = patternPicker_;


- (id)initWithNibName:(NSString *)nibNameOrNil bundle:(NSBundle *)nibBundleOrNil
{
   self = [super initWithNibName:nibNameOrNil bundle:nibBundleOrNil];
   if (self) {
      // Custom initialization
   }
   
   return self;
}

- (void)dealloc
{
   self.tickX = nil;
   self.tickY = nil;
   
   self.gridX = nil;
   self.gridY = nil;
   
   self.logX = nil;
   self.logY = nil;
   self.logZ = nil;
   
   self.colorPicker = nil;
   self.patternPicker = nil;
   
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

@end
