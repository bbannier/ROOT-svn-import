//
//  FileContentController.m
//  root_browser
//
//  Created by Timur Pocheptsov on 8/19/11.
//  Copyright 2011 __MyCompanyName__. All rights reserved.
//

#import <QuartzCore/QuartzCore.h>

#import "FileContentController.h"
#import "ROOTObjectController.h"
#import "SlideshowController.h"
#import "ObjectShortcut.h"
#import "Shortcuts.h"


//C++ (ROOT) imports.
#import "IOSFileContainer.h"
#import "IOSPad.h"

@implementation FileContentController

@synthesize scrollView;

//____________________________________________________________________________________________________
- (id)initWithNibName : (NSString *)nibNameOrNil bundle : (NSBundle *)nibBundleOrNil
{
   self = [super initWithNibName:nibNameOrNil bundle:nibBundleOrNil];

   [self view];

   if (self)
      self.navigationItem.rightBarButtonItem = [[UIBarButtonItem alloc] initWithTitle:@"Slide show" style:UIBarButtonItemStyleBordered target:self action:@selector(startSlideshow)];

   return self;
}

//____________________________________________________________________________________________________
- (void)dealloc
{
   self.scrollView = nil;
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
- (void) correctFrame
{
   CGRect mainFrame;
   CGRect scrollFrame;

   const UIInterfaceOrientation orientation = [UIApplication sharedApplication].statusBarOrientation;
   if (orientation == UIInterfaceOrientationPortrait || orientation == UIInterfaceOrientationPortraitUpsideDown) {
      mainFrame = CGRectMake(0.f, 0.f, 768.f, 1004.f);
      scrollFrame = CGRectMake(0.f, 44.f, 768.f, 960.f);
   } else {
      mainFrame = CGRectMake(0.f, 0.f, 1024.f, 748.f);
      scrollFrame = CGRectMake(0.f, 44.f, 1024.f, 704.f);   
   }
   
   self.view.frame = mainFrame;
   self.scrollView.frame = scrollFrame;
}


//____________________________________________________________________________________________________
- (void)viewDidLoad
{
   [super viewDidLoad];
   [self correctFrame];
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
- (void) clearScrollview
{
   NSArray *viewsToRemove = [self.scrollView subviews];
   for (UIView *v in viewsToRemove)
      [v removeFromSuperview];

}

//____________________________________________________________________________________________________
- (void) initShortcuts
{
   [objectShortcuts release];
   [self clearScrollview];

   objectShortcuts = [[NSMutableArray alloc] init];

   typedef ROOT_iOS::FileContainer::size_type size_type;

   const CGRect rect = CGRectMake(0.f, 0.f, [ObjectShortcut iconWidth], [ObjectShortcut iconHeight]);
   ROOT_iOS::Pad * pad = new ROOT_iOS::Pad(rect.size.width, rect.size.height);//

   for (size_type i = 0; i < fileContainer->GetNumberOfObjects(); ++i) {
      //Create bitmap context.
      UIGraphicsBeginImageContext(rect.size);
      CGContextRef ctx = UIGraphicsGetCurrentContext();
      if (!ctx) {
         //NSLog(@"error while trying to create graphical context for image, iteration %lu", i);
         break;
      }
      //Now draw into this context.
      CGContextTranslateCTM(ctx, 0.f, rect.size.height);
      CGContextScaleCTM(ctx, 1.f, -1.f);
      
      //Fill bitmap with white first.
      CGContextSetRGBFillColor(ctx, 0.f, 0.f, 0.f, 1.f);
      CGContextFillRect(ctx, rect);
      //Set context and paint pad's contents
      //with special colors (color == object's identity)
      pad->cd();
      pad->SetContext(ctx);
      pad->Clear();
      TObject * rootObject = fileContainer->GetObject(i);
      rootObject->Draw(fileContainer->GetDrawOption(i));
      pad->PaintThumbnail();
   
      UIImage *thumbnailImage = UIGraphicsGetImageFromCurrentImageContext();//autoreleased UIImage.
      [thumbnailImage retain];
      UIGraphicsEndImageContext();
       
      NSString *objName = [NSString stringWithFormat : @"%s", rootObject->GetName()];
      ObjectShortcut *shortcut = [[ObjectShortcut alloc] initWithFrame : [ObjectShortcut defaultRect] controller:self objectName:objName thumbnail : thumbnailImage];
      
      shortcut.layer.shadowColor = [UIColor blackColor].CGColor;
      shortcut.layer.shadowOffset = CGSizeMake(20.f, 20.f);
      shortcut.layer.shadowOpacity = 0.3f;
      

      [scrollView addSubview : shortcut];
      [objectShortcuts addObject : shortcut];

      UIBezierPath *path = [UIBezierPath bezierPathWithRect : rect];//shortcut.bounds];
      shortcut.layer.shadowPath = path.CGPath;
      [shortcut release];

      [thumbnailImage release];
   }
      
   delete pad;
}

//____________________________________________________________________________________________________
- (void)didRotateFromInterfaceOrientation:(UIInterfaceOrientation)fromInterfaceOrientation
{
   [self correctFrame];
   [ShorcutUtil placeShortcuts : objectShortcuts inScrollView : scrollView withSize : CGSizeMake([ObjectShortcut iconWidth], [ObjectShortcut iconHeight] + [ObjectShortcut textHeight]) andSpace : 100.f];
}

//____________________________________________________________________________________________________
- (void) activateForFile : (ROOT_iOS::FileContainer *)container
{
   fileContainer = container;
   self.navigationItem.title = [NSString stringWithFormat : @"Contents of %s", container->GetFileName()];
   
   //Prepare objects' thymbnails.
   [self initShortcuts];
   [self correctFrame];
   [ShorcutUtil placeShortcuts : objectShortcuts inScrollView : scrollView withSize : CGSizeMake([ObjectShortcut iconWidth], [ObjectShortcut iconHeight] + [ObjectShortcut textHeight]) andSpace : 100.f];
}

//____________________________________________________________________________________________________
- (void) startSlideshow
{
   if (!slideshowController)
      slideshowController = [[SlideshowController alloc] initWithNibName : @"SlideshowController" bundle : nil];
      
   [self.navigationController pushViewController : slideshowController animated : YES];
}

//____________________________________________________________________________________________________
- (void) selectObjectFromFile : (ObjectShortcut *) obj
{
   if (!objectController)
      objectController = [[ROOTObjectController alloc] initWithNibName:@"ROOTObjectController" bundle : nil];
   
   objectController.navigationItem.title = obj.objectName;
   [self.navigationController pushViewController : objectController animated : YES];
}

@end
