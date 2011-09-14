#import <QuartzCore/QuartzCore.h>

#import "SlideshowController.h"

//C++ (ROOT) imports.
#import "IOSFileContainer.h"
#import "IOSPad.h"

static const CGRect slideViewFrame = CGRectMake(0.f, 0.f, 650.f, 650.f);

namespace {

typedef ROOT_iOS::FileContainer::size_type size_type;

}

@implementation SlideshowController

//____________________________________________________________________________________________________
- (void) correctFramesForOrientation : (UIInterfaceOrientation) orientation
{
   CGRect mainFrame;
   UIInterfaceOrientationIsPortrait(orientation) ? mainFrame = CGRectMake(0.f, 44.f, 768.f, 960.f)
                                                 : (mainFrame = CGRectMake(0.f, 44.f, 1024.f, 704.f));
   parentView.frame = mainFrame;
   CGRect scrollFrame = slideViewFrame;
   scrollFrame.origin.x = mainFrame.size.width / 2 - scrollFrame.size.width / 2;
   scrollFrame.origin.y = mainFrame.size.height / 2 - scrollFrame.size.height / 2;
   scrollView.frame = scrollFrame;
   
   shadowCaster.frame = scrollFrame;
}

//____________________________________________________________________________________________________
- (UIImage *) createImageForObjectAtIndex : (unsigned) objIndex
{
   const CGRect rect = slideViewFrame;
   UIGraphicsBeginImageContext(rect.size);
   CGContextRef ctx = UIGraphicsGetCurrentContext();

   if (!ctx) {
      UIGraphicsEndImageContext();
      return nil;
   }
      
   //Now draw into this context.
   CGContextTranslateCTM(ctx, 0.f, rect.size.height);
   CGContextScaleCTM(ctx, 1.f, -1.f);
      
   //Fill bitmap with white first.
   CGContextSetRGBFillColor(ctx, 1.f, 0.f, 0.f, 1.f);
   CGContextFillRect(ctx, rect);
   //Set context and paint pad's contents
   //with special colors (color == object's identity)
   pad->cd();
   pad->SetContext(ctx);
   pad->SetViewWH(rect.size.width, rect.size.height);
   pad->Clear();
   fileContainer->GetObject(objIndex)->Draw(fileContainer->GetDrawOption(objIndex));
   pad->Paint();
   
   UIImage *image = UIGraphicsGetImageFromCurrentImageContext();//autoreleased UIImage.
   [image retain];
   UIGraphicsEndImageContext();
       
   return image;
}

//____________________________________________________________________________________________________
- (void) setAdjacentIndices
{
   //Pre-condition: fileContainer has at least one object inside.
   currIndex > 0 ? prevIndex = currIndex - 1 : prevIndex = fileContainer->GetNumberOfObjects() - 1;
   currIndex + 1 == fileContainer->GetNumberOfObjects() ? nextIndex = 0 : nextIndex = currIndex + 1;
}

//____________________________________________________________________________________________________
- (void) initPadViewsAndScroll
{
   CGRect padFrame = slideViewFrame;

   if (fileContainer->GetNumberOfObjects() == 1) {
      //This is the only special case.
      pad = new ROOT_iOS::Pad(slideViewFrame.size.width, slideViewFrame.size.height);
      padImageViews[0] = [[UIImageView alloc] initWithFrame : padFrame];
      [scrollView addSubview : padImageViews[0]];
      [padImageViews[0] release];
      scrollView.contentSize = padFrame.size;
   } else {
      pad = new ROOT_iOS::Pad(slideViewFrame.size.width, slideViewFrame.size.height);

      for (unsigned i = 0; i < 3; ++i) {
         padFrame.origin.x = i * padFrame.size.width;

         padImageViews[i] = [[UIImageView alloc] initWithFrame : padFrame];
         [scrollView addSubview : padImageViews[i]];
         [padImageViews[i] release];
      }
      
      scrollView.contentSize = CGSizeMake(3 * padFrame.size.width, padFrame.size.height);
      [scrollView scrollRectToVisible:padImageViews[1].frame animated : NO];

   }
}

//____________________________________________________________________________________________________
- (void) loadPagesForCurrentIndex
{
   //Pre-conditions: file container must have at least one object;
   //pad(s) and view(s) must be initialized already.
   
   if (fileContainer->GetNumberOfObjects() == 1) {
      UIImage *image = [self createImageForObjectAtIndex : 0];
      padImageViews[0].image = image;
      [image release];
   } else {
      UIImage *image = [self createImageForObjectAtIndex : prevIndex];
      padImageViews[0].image = image;
      [image release];
      
      image = [self createImageForObjectAtIndex : currIndex];
      padImageViews[1].image = image;
      [image release];
      
      image = [self createImageForObjectAtIndex : nextIndex];
      padImageViews[2].image = image;
      [image release];
   }
}

//____________________________________________________________________________________________________
- (id)initWithNibName : (NSString *)nibNameOrNil bundle : (NSBundle *)nibBundleOrNil fileContainer : (ROOT_iOS::FileContainer *)container
{
   self = [super initWithNibName:nibNameOrNil bundle:nibBundleOrNil];

   [self view];

   if (self) {
      fileContainer = container;

      if (fileContainer->GetNumberOfObjects()) {
         [self initPadViewsAndScroll];

         prevIndex = currIndex = nextIndex = 0;//This is done by runtime already.
         [self setAdjacentIndices];
         [self loadPagesForCurrentIndex];
         
         scrollView.canCancelContentTouches = YES;
         scrollView.delaysContentTouches = YES;
         
      }

      [parentView bringSubviewToFront : scrollView];

      [self correctFramesForOrientation : self.interfaceOrientation];

      //Many thanks to Apple - I need this stupid view, because 
      //UIScrollView does not cast shadow. Why? May be Jobs "think different" knows?
      shadowCaster.layer.shadowColor = [UIColor blackColor].CGColor;
      shadowCaster.layer.shadowOpacity = 0.3f;
      shadowCaster.layer.shadowOffset = CGSizeMake(10.f, 10.f);
      shadowCaster.layer.shadowPath = [UIBezierPath bezierPathWithRect : slideViewFrame].CGPath;
   }

   return self;
}

//____________________________________________________________________________________________________
- (void) scrollPagesLeft
{
   //Shift 'current' and 'next' image to the left (making them 'prev' and 'current')
   //and load a new image into the 'next'.
   UIImage *prevImage = padImageViews[1].image;
   [prevImage retain];
   UIImage *currImage = padImageViews[2].image;
   [currImage retain];
   
   padImageViews[0].image = prevImage; //And delete old 'previous' image.
   padImageViews[1].image = currImage;
   
   [prevImage release];
   [currImage release];
   
   //Draw only one new object.
   UIImage *nextImage = [self createImageForObjectAtIndex : nextIndex];
   padImageViews[2].image = nextImage;
   [nextImage release];
}

//____________________________________________________________________________________________________
- (void) scrollPagesRight
{
   //Shift 'previous' and 'current' image to right (making them 'current' and 'next')
   //and load a new image into 'previous'.
   UIImage *currImage = padImageViews[0].image;
   [currImage retain];
   UIImage *nextImage = padImageViews[1].image;
   [nextImage retain];
   
   padImageViews[1].image = currImage;
   padImageViews[2].image = nextImage;//And delete old 'next' image.
   
   [currImage release];
   [nextImage release];
   
   UIImage *prevImage = [self createImageForObjectAtIndex : prevIndex];
   padImageViews[0].image = prevImage;
   [prevImage release];
}

//____________________________________________________________________________________________________
- (void) scrollViewDidEndDecelerating : (UIScrollView *) sender 
{
   if (scrollView.contentOffset.x > scrollView.frame.size.width) {
      currIndex + 1 == fileContainer->GetNumberOfObjects() ? currIndex = 0 : ++currIndex;
      [self setAdjacentIndices];
      [self scrollPagesLeft];
   } else if (scrollView.contentOffset.x < scrollView.frame.size.width) {
      currIndex > 0 ? --currIndex : currIndex = fileContainer->GetNumberOfObjects() - 1;
      [self setAdjacentIndices];
      [self scrollPagesRight];
   }
   
   [scrollView scrollRectToVisible:CGRectMake(slideViewFrame.size.width, 0.f, slideViewFrame.size.width, slideViewFrame.size.height) animated : NO];
}

//____________________________________________________________________________________________________
- (void)dealloc
{
   delete pad;

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
- (void)willAnimateRotationToInterfaceOrientation : (UIInterfaceOrientation)interfaceOrientation duration : (NSTimeInterval)duration {
   [self correctFramesForOrientation : interfaceOrientation];
}

@end
