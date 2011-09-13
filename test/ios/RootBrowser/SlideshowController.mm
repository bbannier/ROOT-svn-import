#import <QuartzCore/QuartzCore.h>

#import "SlideshowController.h"
#import "SlideView.h"

//C++ (ROOT) imports.
#import "IOSFileContainer.h"
#import "IOSPad.h"

static const CGRect slideViewFrame = CGRectMake(0.f, 0.f, 700.f, 700.f);

@implementation SlideshowController

//____________________________________________________________________________________________________
- (void) correctFramesForOrientation : (UIInterfaceOrientation) orientation
{
   CGRect mainFrame;
   UIInterfaceOrientationIsPortrait(orientation) ? mainFrame = CGRectMake(0.f, 44.f, 768.f, 960.f)
                                                 : (mainFrame = CGRectMake(0.f, 44.f, 1024.f, 704.f));
   
   self.view.frame = mainFrame;
   for (unsigned i = 0; i < 2; ++i) {
      //Many thanks to Apple's programmers - I can not assing, say, rect.origin, I have to write all these ugly expressions here.
      slides[i].frame = CGRectMake(mainFrame.size.width / 2 - slideViewFrame.size.width / 2, 44 + mainFrame.size.height / 2 - slideViewFrame.size.height / 2, slideViewFrame.size.width, slideViewFrame.size.height);
   }
}

//____________________________________________________________________________________________________
- (void) animateSlides : (NSString *)direction
{
   SlideView *viewToHide = slides[visiblePad ? 0 : 1];
   SlideView *viewToShow = slides[visiblePad ? 1 : 0];
   viewToHide.hidden = YES;
   viewToShow.hidden = NO;
   [viewToShow setNeedsDisplay];
   
   // First create a CATransition object to describe the transition
   CATransition *transition = [CATransition animation];
   // Animate over 3/4 of a second
   transition.duration = 0.5;
   // using the ease in/out timing function
   transition.timingFunction = [CAMediaTimingFunction functionWithName:kCAMediaTimingFunctionEaseInEaseOut];
   // Now to set the type of transition.
   transition.type = kCATransitionMoveIn;// kCATransitionReveal;
   transition.subtype = direction;//kCATransitionFromRight;

   // Finally, to avoid overlapping transitions we assign ourselves as the delegate for the animation and wait for the
   // -animationDidStop:finished: message. When it comes in, we will flag that we are no longer transitioning.
   //transitioning = YES;
   transition.delegate = self;
   // Next add it to the containerView's layer. This will perform the transition based on how we change its contents.
   [self.view.layer addAnimation : transition forKey : nil];
}

//____________________________________________________________________________________________________
- (void) handleSwipe : (UISwipeGestureRecognizer *)swipe
{
   if (swipe.direction == UISwipeGestureRecognizerDirectionLeft) {
      if (nCurrentObject + 1 < fileContainer->GetNumberOfObjects()) {
         visiblePad = visiblePad ? 0 : 1;
         ++nCurrentObject;
         pads[visiblePad]->Clear();
         pads[visiblePad]->cd();
         fileContainer->GetObject(nCurrentObject)->Draw(fileContainer->GetDrawOption(nCurrentObject));
         [self animateSlides : kCATransitionFromRight];
      }
   } else if (swipe.direction == UISwipeGestureRecognizerDirectionRight) {
      if (nCurrentObject) {
         visiblePad = visiblePad ? 0 : 1;
         --nCurrentObject;
         pads[visiblePad]->Clear();
         pads[visiblePad]->cd();
         fileContainer->GetObject(nCurrentObject)->Draw(fileContainer->GetDrawOption(nCurrentObject));
         [self animateSlides : kCATransitionFromLeft];
      }
   }
}

//____________________________________________________________________________________________________
- (id)initWithNibName : (NSString *)nibNameOrNil bundle : (NSBundle *)nibBundleOrNil fileContainer : (ROOT_iOS::FileContainer *)container
{
   self = [super initWithNibName:nibNameOrNil bundle:nibBundleOrNil];

   if (self) {
      fileContainer = container;

      for (unsigned i = 0; i < 2; ++i) {
         pads[i] = new ROOT_iOS::Pad(slideViewFrame.size.width, slideViewFrame.size.height);

         slides[i] = [[SlideView alloc] initWithFrame:slideViewFrame andPad:pads[i]];
         [self.view addSubview : slides[i]];
         [slides[i] release];
         slides[i].hidden = YES;
      }

      if (fileContainer->GetNumberOfObjects()) {
         pads[0]->cd();
         fileContainer->GetObject(0)->Draw(fileContainer->GetDrawOption(0));
         slides[0].hidden = NO;
      }

      UISwipeGestureRecognizer *swipe = [[UISwipeGestureRecognizer alloc] initWithTarget : self action : @selector(handleSwipe:)];
      [self.view addGestureRecognizer : swipe];
      swipe.direction =  UISwipeGestureRecognizerDirectionLeft;
      [swipe release];
      
      //Now, many thanks to Apple's brilliant programmers, I have to create another swipe recognizer.
      swipe = [[UISwipeGestureRecognizer alloc] initWithTarget : self action : @selector(handleSwipe:)];
      swipe.direction = UISwipeGestureRecognizerDirectionRight;
      [self.view addGestureRecognizer : swipe];
      [swipe release];

      [self correctFramesForOrientation : self.interfaceOrientation];
   }

   return self;
}

//____________________________________________________________________________________________________
- (void)dealloc
{
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
