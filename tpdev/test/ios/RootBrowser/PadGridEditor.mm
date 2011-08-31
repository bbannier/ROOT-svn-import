//
//  PadGridEditor.m
//  editors
//
//  Created by Timur Pocheptsov on 8/16/11.
//  Copyright 2011 __MyCompanyName__. All rights reserved.
//
#import "ROOTObjectController.h"
#import "PadGridEditor.h"

#import "TVirtualPad.h"
#import "TObject.h"

@implementation PadGridEditor

- (id)initWithNibName:(NSString *)nibNameOrNil bundle:(NSBundle *)nibBundleOrNil
{
   self = [super initWithNibName:nibNameOrNil bundle:nibBundleOrNil];

   if (self) {
   }
   
   return self;
}

- (void)dealloc
{
   [super dealloc];
}

- (void) setObject : (TObject *)o
{
   object = static_cast<TVirtualPad *>(o);
}

- (void) setController : (ROOTObjectController *)c
{
   controller = c;
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

#pragma mark - Editor's actions.

- (IBAction) gridActivated : (UISwitch *) g
{
   if (g == gridX)
      object->SetGridx(g.on);
   else if (g == gridY)
      object->SetGridy(g.on);
   
   [controller objectWasModifiedByEditor];
}

- (IBAction) ticksActivated : (UISwitch *) t
{
   if (t == ticksX)
      object->SetTickx(t.on);
   else if (t == ticksY)
      object->SetTicky(t.on);
   
   [controller objectWasModifiedByEditor];
}


@end
