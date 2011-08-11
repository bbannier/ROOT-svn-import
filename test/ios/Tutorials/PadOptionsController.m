//
//  PadOptionsController.m
//  Tutorials
//
//  Created by Timur Pocheptsov on 8/11/11.
//  Copyright 2011 __MyCompanyName__. All rights reserved.
//

#import "PadOptionsController.h"
#import "ColorCell.h"

static const double predefinedFillColors[16][3] = 
{
{1., 1., 1.},
{0., 0., 0.},
{251 / 255., 0., 24 / 255.},
{40 / 255., 253 / 255., 44 / 255.},
{31 / 255., 29 / 255., 251 / 255.},
{253 / 255., 254 / 255., 52 / 255.},
{253 / 255., 29 / 255., 252 / 255.},
{53 / 255., 1., 254 / 255.},
{94 / 255., 211 / 255., 90 / 255.},
{92 / 255., 87 / 255., 214 / 255.},
{135 / 255., 194 / 255., 164 / 255.},
{127 / 255., 154 / 255., 207 / 255.},
{211 / 255., 206 / 255., 138 / 255.},
{220 / 255., 185 / 255., 138 / 255.},
{209 / 255., 89 / 255., 86 / 255.},
{147 / 255., 29 / 255., 251 / 255.}
};

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
@synthesize colors = colors_;

- (id)initWithNibName:(NSString *)nibNameOrNil bundle:(NSBundle *)nibBundleOrNil
{
   self = [super initWithNibName:nibNameOrNil bundle:nibBundleOrNil];
   if (self) {
      // Custom initialization
      colors_ = [[NSMutableArray alloc] init];
      for (unsigned i = 0; i < 16; ++i) {
         ColorCell * newCell = [[ColorCell alloc] initWithFrame : CGRectMake(0.f, 0.f, 80.f, 44.f)];
         [newCell setColorID : i andRGB : predefinedFillColors[i]];
         [colors_ addObject : newCell];
         [newCell release];
      }
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
   
   [colors_ release];
   
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

#pragma mark - color picker delegate.
- (CGFloat)pickerView:(UIPickerView *)pickerView widthForComponent:(NSInteger)component
{
/*   if (pickerView == colorPicker_)
      return 80;
   }
   
	return [CustomView viewWidth];*/
   return 80.;
}

- (CGFloat)pickerView:(UIPickerView *)pickerView rowHeightForComponent:(NSInteger)component
{
	//return [CustomView viewHeight];
   return 44.;
}

- (NSInteger)pickerView:(UIPickerView *)pickerView numberOfRowsInComponent:(NSInteger)component
{
   if (pickerView == colorPicker_)
      return [colors_ count];
   
   return 0;
}

- (NSInteger)numberOfComponentsInPickerView:(UIPickerView *)pickerView
{
	return 1;
}


#pragma mark -
#pragma mark UIPickerViewDelegate

// tell the picker which view to use for a given component and row, we have an array of views to show
- (UIView *)pickerView:(UIPickerView *)pickerView viewForRow:(NSInteger)row
		  forComponent:(NSInteger)component reusingView:(UIView *)view
{
   if (pickerView == colorPicker_) {
      return [colors_ objectAtIndex : row];
   }

   return 0;
}

- (void)pickerView:(UIPickerView *)thePickerView didSelectRow:(NSInteger)row inComponent:(NSInteger)component {
   NSLog(@"Selected Color: %d.", row);
}

@end
