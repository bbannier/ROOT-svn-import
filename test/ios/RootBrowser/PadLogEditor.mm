//
//  PadLogEditor.m
//  editors
//
//  Created by Timur Pocheptsov on 8/16/11.
//  Copyright 2011 __MyCompanyName__. All rights reserved.
//

#import "ROOTObjectController.h"
#import "PadLogEditor.h"

//C++ (ROOT) imports.
#import "TVirtualPad.h"
#import "TObject.h"


@implementation PadLogEditor

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
    [super dealloc];
}

- (void) setController : (ROOTObjectController *)c
{
   controller = c;
}

- (void) setObject : (TObject *)o
{
   object = static_cast<TVirtualPad *>(o);
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

- (IBAction) logActivated : (UISwitch *) log
{
   if (log == logX)
      object->SetLogx(log.on);
   if (log == logY)
      object->SetLogy(log.on);
   if (log == logZ)
      object->SetLogz(log.on);
   
   [controller objectWasModifiedByEditor];
}

@end
