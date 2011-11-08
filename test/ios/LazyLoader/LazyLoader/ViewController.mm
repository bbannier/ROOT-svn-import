#import "ViewController.h"
#import "ObjectShortcut.h"
#import "ThumbnailView.h"
#import "ImageLoader.h"

#import "FileUtils.h"

#import "TObject.h"
#import "IOSPad.h"

@implementation ViewController {
   ROOT::iOS::Browser::FileContainer *fileContainer;
   ThumbnailView *thumbnailView;
   NSMutableSet *imageCache;
   
   NSOperationQueue *queue;
}

//____________________________________________________________________________________________________
- (void) didReceiveMemoryWarning
{
   [super didReceiveMemoryWarning];
   // Release any cached data, images, etc that aren't in use.
}

#pragma mark - View lifecycle

//____________________________________________________________________________________________________
- (void) viewDidLoad
{
   [super viewDidLoad];
	// Do any additional setup after loading the view, typically from a nib.
   
   using namespace ROOT::iOS::Browser;
   
   NSString *filePath = [[NSBundle mainBundle] pathForResource : @"mo" ofType : @"root"];
   if (filePath) {
      fileContainer = CreateROOTFileContainer("http://root.cern.ch/files/ios/mo.root");
      if (fileContainer) {
         NSLog(@"Object keys read from file %lu", fileContainer->GetNumberOfObjects());

         thumbnailView = [[ThumbnailView alloc] initWithFrame : self.view.bounds];
         thumbnailView.contentMode = UIViewContentModeScaleToFill;
         thumbnailView.autoresizingMask = UIViewAutoresizingFlexibleHeight | UIViewAutoresizingFlexibleWidth | UIViewAutoresizingFlexibleLeftMargin |
                                          UIViewAutoresizingFlexibleRightMargin | UIViewAutoresizingFlexibleTopMargin | UIViewAutoresizingFlexibleBottomMargin;
         [self.view addSubview : thumbnailView];
         thumbnailView.itemSize = CGSizeMake(150.f, 180.f);
         thumbnailView.addW = 20.f;
         thumbnailView.addH = 20.f;
         thumbnailView.addSide = 30.f;
         thumbnailView.delegate = self;
         [thumbnailView loadData];
      }
   }
   
   imageCache = [[NSMutableSet alloc] init];
}

//____________________________________________________________________________________________________
- (void) dealloc
{
   DeleteROOTFileContainer(fileContainer);
}

//____________________________________________________________________________________________________
- (void) viewDidUnload
{
    [super viewDidUnload];
    // Release any retained subviews of the main view.
    // e.g. self.myOutlet = nil;
}

//____________________________________________________________________________________________________
- (void) viewWillAppear : (BOOL)animated
{
   [super viewWillAppear:animated];
}

//____________________________________________________________________________________________________
- (void) viewDidAppear : (BOOL)animated
{
   [super viewDidAppear : animated];
}

//____________________________________________________________________________________________________
- (void) viewWillDisappear : (BOOL)animated
{
	[super viewWillDisappear:animated];
}

//____________________________________________________________________________________________________
- (void) viewDidDisappear : (BOOL)animated
{
	[super viewDidDisappear:animated];
}

//____________________________________________________________________________________________________
- (BOOL) shouldAutorotateToInterfaceOrientation:(UIInterfaceOrientation)interfaceOrientation
{
    // Return YES for supported orientations
   return YES;
}

//____________________________________________________________________________________________________
- (void) loadThumbnailForView : (UIView *)view 
{
   using namespace ROOT::iOS;
   
   if (fileContainer->GetObject(view.tag)) {
      Pad *pad = fileContainer->GetPadAttached(view.tag);
      //generate thumbnail.
      
      const CGRect rect = CGRectMake(0.f, 0.f, [ObjectShortcut imageWidth], [ObjectShortcut imageHeight]);
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
      pad->SetViewWH(rect.size.width, rect.size.height);
      pad->SetContext(ctx);
      pad->PaintThumbnail();


      ObjectShortcut *sh = (ObjectShortcut *)view;
      sh.image = UIGraphicsGetImageFromCurrentImageContext();
   
      UIGraphicsEndImageContext();
   }
}

- (unsigned) numberOfThumbnailsInView : (ThumbnailView *)view
{
   if (fileContainer)
      return fileContainer->GetNumberOfObjects();
   return 0;
}

- (UIView *) thumbnailAtIndex : (unsigned)index
{
   if (!fileContainer)
      return nil;

   ObjectShortcut *shortcut = (ObjectShortcut *)[thumbnailView getThumbnailFromCache];
   if (!shortcut) {
      CGRect frame = CGRectMake(0.f, 0.f, [ObjectShortcut imageWidth], [ObjectShortcut imageHeight] + 30);
      shortcut = [[ObjectShortcut alloc] initWithFrame:frame];
   }
   
   [shortcut setObjectType : fileContainer->GetObjectType(index)];
   [shortcut setObjectName : [NSString stringWithFormat : @"%s", fileContainer->GetObjectName(index)]];
   return shortcut;
}


- (void) cacheDataForThumbnail : (UIView *)view
{
   ((ObjectShortcut *)view).image = nil;
}

- (void) loadDataForVisibleRange : (NSMutableSet *)range
{

   /*queue = [[NSOperationQueue alloc] init];
   NSMutableSet *copy = [range copy];
   ImageLoader *loader = [[ImageLoader alloc] initWithContainer : fileContainer andViews : copy];
   [queue addOperation : loader];*/
   
   for (UIView *v in range) {
      [self loadThumbnailForView : v];
      [v setNeedsDisplay];
   }
}

- (void) cancelDataLoad
{
 //  [queue cancelAllOperations];
}


@end
