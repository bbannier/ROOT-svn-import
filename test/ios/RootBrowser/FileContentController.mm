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
@synthesize fileContainer;

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
- (void) correctFramesForOrientation : (UIInterfaceOrientation) orientation
{
   CGRect mainFrame;
   CGRect scrollFrame;

   if (orientation == UIInterfaceOrientationPortrait || orientation == UIInterfaceOrientationPortraitUpsideDown) {
      mainFrame = CGRectMake(0.f, 0.f, 768.f, 1004.f);
      scrollFrame = CGRectMake(0.f, 44.f, 768.f, 960.f);
   } else {
      mainFrame = CGRectMake(0.f, 0.f, 1024.f, 748.f);
      scrollFrame = CGRectMake(0.f, 44.f, 1024.f, 704.f);   
   }
   
   self.view.frame = mainFrame;
   self.scrollView.frame = scrollFrame;
   
   if ([[scrollView subviews] count])
      [ShorcutUtil placeShortcuts : objectShortcuts inScrollView : scrollView withSize : CGSizeMake([ObjectShortcut iconWidth], [ObjectShortcut iconHeight] + [ObjectShortcut textHeight]) andSpace : 100.f];
}

//____________________________________________________________________________________________________
- (void) viewWillAppear:(BOOL)animated
{
   //self.interfaceOrientation ?
   [self correctFramesForOrientation : [UIApplication sharedApplication].statusBarOrientation];
}

//____________________________________________________________________________________________________
- (void)viewDidLoad
{
   [super viewDidLoad];
//   [self correctFrames];
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
- (void)willAnimateRotationToInterfaceOrientation : (UIInterfaceOrientation)interfaceOrientation duration:(NSTimeInterval)duration {
    [self correctFramesForOrientation : interfaceOrientation];
}

//____________________________________________________________________________________________________
- (void) clearScrollview
{
   NSArray *viewsToRemove = [self.scrollView subviews];
   for (UIView *v in viewsToRemove)
      [v removeFromSuperview];

}

//____________________________________________________________________________________________________
- (void) addShortcutForObjectAtIndex : (unsigned) objIndex inPad : (ROOT_iOS::Pad *) pad
{
   const CGRect rect = CGRectMake(0.f, 0.f, [ObjectShortcut iconWidth], [ObjectShortcut iconHeight]);
   UIGraphicsBeginImageContext(rect.size);
   CGContextRef ctx = UIGraphicsGetCurrentContext();
   if (!ctx) {
      UIGraphicsEndImageContext();
      return;
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
   fileContainer->GetObject(objIndex)->Draw(fileContainer->GetDrawOption(objIndex));
   pad->PaintThumbnail();
   
   UIImage *thumbnailImage = UIGraphicsGetImageFromCurrentImageContext();//autoreleased UIImage.
   [thumbnailImage retain];
   UIGraphicsEndImageContext();
       
   ObjectShortcut *shortcut = [[ObjectShortcut alloc] initWithFrame : [ObjectShortcut defaultRect] controller : self forObjectAtIndex:objIndex withThumbnail:thumbnailImage];
   shortcut.layer.shadowColor = [UIColor blackColor].CGColor;
   shortcut.layer.shadowOffset = CGSizeMake(20.f, 20.f);
   shortcut.layer.shadowOpacity = 0.3f;

   [scrollView addSubview : shortcut];
   [objectShortcuts addObject : shortcut];

   UIBezierPath *path = [UIBezierPath bezierPathWithRect : rect];
   shortcut.layer.shadowPath = path.CGPath;
   [shortcut release];

   [thumbnailImage release];
}

//____________________________________________________________________________________________________
- (void) addObjectsIntoScrollview
{
   typedef ROOT_iOS::FileContainer::size_type size_type;

   [objectShortcuts release];
   [self clearScrollview];

   objectShortcuts = [[NSMutableArray alloc] init];

   const CGRect rect = CGRectMake(0.f, 0.f, [ObjectShortcut iconWidth], [ObjectShortcut iconHeight]);
   ROOT_iOS::Pad * pad = new ROOT_iOS::Pad(rect.size.width, rect.size.height);//Pad to draw object.

   for (size_type i = 0; i < fileContainer->GetNumberOfObjects(); ++i)
      [self addShortcutForObjectAtIndex : i inPad : pad];

   delete pad;
}

//____________________________________________________________________________________________________
- (void) activateForFile : (ROOT_iOS::FileContainer *)container
{
   fileContainer = container;
   self.navigationItem.title = [NSString stringWithFormat : @"Contents of %s", container->GetFileName()];
   
   //Prepare objects' thymbnails.
   [self addObjectsIntoScrollview];
   [self correctFramesForOrientation : [UIApplication sharedApplication].statusBarOrientation];
   //[ShorcutUtil placeShortcuts : objectShortcuts inScrollView : scrollView withSize : CGSizeMake([ObjectShortcut iconWidth], [ObjectShortcut iconHeight] + [ObjectShortcut textHeight]) andSpace : 100.f];
}

//____________________________________________________________________________________________________
- (void) startSlideshow
{
   slideshowController = [[SlideshowController alloc] initWithNibName : @"SlideshowController" bundle : nil fileContainer : fileContainer];
   [self.navigationController pushViewController : slideshowController animated : YES];
   [slideshowController release];
}

//____________________________________________________________________________________________________
- (void) selectObjectFromFile : (ObjectShortcut *) shortcut
{
   objectController = [[ROOTObjectController alloc] initWithNibName:@"ROOTObjectController" bundle : nil];
//   objectController.navigationItem.title = [NSString stringWithFormat : @"%s", shortcut.objectName];
   [objectController setObjectWithIndex : shortcut.objectIndex fromContainer : fileContainer];
   [self.navigationController pushViewController : objectController animated : YES];
}

@end
