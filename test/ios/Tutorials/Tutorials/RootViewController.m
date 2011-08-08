//
//  RootViewController.m
//  Tutorials
//
//  Created by Timur Pocheptsov on 7/7/11.
//  Copyright 2011 __MyCompanyName__. All rights reserved.
//

#import <Foundation/NSTimer.h>

#import "RootViewController.h"

#import "DetailViewController.h"

#import "ExclusionGraphDemo.h"
#import "PolarGraphDemo.h"
#import "EuropeMapDemo.h"
#import "HsimpleDemo.h"
#import "SurfaceDemo.h"
#import "LegoDemo.h"

@implementation RootViewController
		
@synthesize detailViewController;

//_________________________________________________________________
- (void)viewDidLoad
{
   tutorialNames = [[NSMutableArray alloc] init];
   [tutorialNames addObject:@"Hsimple"];
   [tutorialNames addObject:@"Surface"];
   [tutorialNames addObject:@"Polar graph"];
   [tutorialNames addObject:@"Lego"];
   [tutorialNames addObject:@"Exclusion graph"];
   [tutorialNames addObject:@"TH2Poly"];
   
   tutorialIcons = [[NSMutableArray alloc] init];
   [tutorialIcons addObject:@"hsimple_icon_n.png"];
   [tutorialIcons addObject:@"surface_icon_n.png"];
   [tutorialIcons addObject:@"polar_icon_n.png"];
   [tutorialIcons addObject:@"lego_icon_n.png"];
   [tutorialIcons addObject:@"exclusion_icon_n.png"];
   [tutorialIcons addObject:@"europe_icon.png"];

   //Set table view's color, row height, separator's color
   //(I want separator to be invisible).
   self.tableView.rowHeight = 72.f;
   self.tableView.backgroundColor = [UIColor lightGrayColor];
   self.tableView.separatorColor = [UIColor lightGrayColor];

   //This code was generated by ide.
   [super viewDidLoad];
   self.clearsSelectionOnViewWillAppear = NO;
   self.contentSizeForViewInPopover = CGSizeMake(320.0, 500.0);//Default height was 600., I've changed it to 500.
   
   demos[0] = new ROOT_iOS::Demos::HsimpleDemo;
   demos[1] = new ROOT_iOS::Demos::SurfaceDemo;
   demos[2] = new ROOT_iOS::Demos::PolarGraphDemo;
   demos[3] = new ROOT_iOS::Demos::LegoDemo;
   demos[4] = new ROOT_iOS::Demos::ExclusionGraphDemo;
   demos[5] = new ROOT_iOS::Demos::EuropeMapDemo;
}

//_________________________________________________________________
- (void)viewWillAppear:(BOOL)animated
{
   [super viewWillAppear:animated];
}

//_________________________________________________________________
- (void)viewDidAppear:(BOOL)animated
{
   [super viewDidAppear:animated];
}

//_________________________________________________________________
- (void)viewWillDisappear:(BOOL)animated
{
	[super viewWillDisappear:animated];
}

//_________________________________________________________________
- (void)viewDidDisappear:(BOOL)animated
{
	[super viewDidDisappear:animated];
}

//_________________________________________________________________
- (BOOL)shouldAutorotateToInterfaceOrientation:(UIInterfaceOrientation)interfaceOrientation
{
   return YES;
}

//_________________________________________________________________
- (NSInteger)numberOfSectionsInTableView:(UITableView *)tableView
{
   return 1;
   		
}

//_________________________________________________________________
- (NSInteger)tableView:(UITableView *)tableView numberOfRowsInSection:(NSInteger)section
{
   return [tutorialNames count];   		
}

//_________________________________________________________________
- (UITableViewCell *)tableView:(UITableView *)tableView cellForRowAtIndexPath:(NSIndexPath *)indexPath
{
   static NSString *CellIdentifier = @"Cell";
   
   UITableViewCell *cell = [tableView dequeueReusableCellWithIdentifier:CellIdentifier];
   if (cell == nil)
      cell = [[[UITableViewCell alloc] initWithStyle:UITableViewCellStyleDefault reuseIdentifier:CellIdentifier] autorelease];

   // Configure the cell.
   const BOOL useDarkBackground = indexPath.row % 2;
   NSString *backgroundImagePath = [[NSBundle mainBundle] pathForResource:useDarkBackground ? @"DarkBackground" : @"LightBackground" ofType:@"png"];

   UIImage *backgroundImage = [[UIImage imageWithContentsOfFile:backgroundImagePath] stretchableImageWithLeftCapWidth : 0.f topCapHeight : 1.f];   
   cell.backgroundView = [[[UIImageView alloc] initWithImage:backgroundImage] autorelease];
   cell.backgroundView.autoresizingMask = UIViewAutoresizingFlexibleWidth | UIViewAutoresizingFlexibleHeight;
   cell.backgroundView.frame = cell.bounds;
  
   cell.textLabel.text = [tutorialNames objectAtIndex:indexPath.row];
   cell.imageView.image = [UIImage imageNamed: [tutorialIcons objectAtIndex:indexPath.row]];

   return cell;
}

/*
// Override to support conditional editing of the table view.
- (BOOL)tableView:(UITableView *)tableView canEditRowAtIndexPath:(NSIndexPath *)indexPath
{
    // Return NO if you do not want the specified item to be editable.
    return YES;
}
*/

/*
// Override to support editing the table view.
- (void)tableView:(UITableView *)tableView commitEditingStyle:(UITableViewCellEditingStyle)editingStyle forRowAtIndexPath:(NSIndexPath *)indexPath
{

    if (editingStyle == UITableViewCellEditingStyleDelete) {
        // Delete the row from the data source.
        [tableView deleteRowsAtIndexPaths:[NSArray arrayWithObject:indexPath] withRowAnimation:UITableViewRowAnimationFade];
    }   
    else if (editingStyle == UITableViewCellEditingStyleInsert) {
        // Create a new instance of the appropriate class, insert it into the array, and add a new row to the table view.
    }   
}
*/

/*
// Override to support rearranging the table view.
- (void)tableView:(UITableView *)tableView moveRowAtIndexPath:(NSIndexPath *)fromIndexPath toIndexPath:(NSIndexPath *)toIndexPath
{
}
*/

//_________________________________________________________________
- (void)tableView:(UITableView *)tableView didSelectRowAtIndexPath:(NSIndexPath *)indexPath
{
    // Navigation logic may go here -- for example, create and push another view controller.
    /*
     <#DetailViewController#> *detailViewController = [[<#DetailViewController#> alloc] initWithNibName:@"<#Nib name#>" bundle:nil];
     NSManagedObject *selectedObject = [[self fetchedResultsController] objectAtIndexPath:indexPath];
     // ...
     // Pass the selected object to the new view controller.
     [self.navigationController pushViewController:detailViewController animated:YES];
     [detailViewController release];
     */

   //My code to be replaced.
   detailViewController.detailItem = 0;//This will call setDetailItem and "dissolve" popover.
   [detailViewController setActiveDemo:demos[indexPath.row]];
}

//_________________________________________________________________
- (void) onTimer
{
   demos[0]->NextStep();
}

//_________________________________________________________________
- (void)didReceiveMemoryWarning
{
   // Releases the view if it doesn't have a superview.
   [super didReceiveMemoryWarning];

   // Relinquish ownership any cached data, images, etc that aren't in use.
}

//_________________________________________________________________
- (void)viewDidUnload
{
   // Relinquish ownership of anything that can be recreated in viewDidLoad or on demand.
   // For example: self.myOutlet = nil;
}

//_________________________________________________________________
- (void)dealloc
{
   [detailViewController release];
   [tutorialNames release];
   [tutorialIcons release];
   
   [super dealloc];
}

//_________________________________________________________________
- (void)tableView:(UITableView *)tableView willDisplayCell:(UITableViewCell *)cell forRowAtIndexPath:(NSIndexPath *)indexPath
{
   [[cell textLabel] setBackgroundColor:[UIColor clearColor]];
   [[cell detailTextLabel] setBackgroundColor:[UIColor clearColor]];
}

@end
