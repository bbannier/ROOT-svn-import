//
//  ROOTObjectController.m
//  root_browser
//
//  Created by Timur Pocheptsov on 8/19/11.
//  Copyright 2011 __MyCompanyName__. All rights reserved.
//

#import <QuartzCore/QuartzCore.h>

#import "ScrollViewWithPadView.h"
#import "ROOTObjectController.h"
#import "ObjectShortcut.h"
#import "PadGridEditor.h"
#import "PadLogEditor.h"
#import "FillEditor.h"
#import "EditorView.h"
#import "PadView.h"

//C++ (ROOT) imports.
#import "TObject.h"
#import "IOSPad.h"

@implementation ROOTObjectController

@synthesize scrollView;

//____________________________________________________________________________________________________
- (void) correctPadFrameNoEditor : (UIInterfaceOrientation) orientation
{
   CGRect padFrame = padView.frame;
   
   padFrame.size.width = 600.f;
   padFrame.size.height = 600.f;
   
   if (UIInterfaceOrientationIsPortrait(orientation)) {
      padFrame.origin.x = 384.f - padFrame.size.width / 2;
      padFrame.origin.y = 480.f - padFrame.size.height / 2;
   } else {
      padFrame.origin.x = 512.f - padFrame.size.width / 2;
      padFrame.origin.y = 352.f - padFrame.size.height / 2;
   }
   
   padView.frame = padFrame;
}

//____________________________________________________________________________________________________
- (void) correctPadFrameWithEditor : (UIInterfaceOrientation) orientation
{
   //The most tricky part, since this code can be called
   //for animation.
   CGRect padFrame = padView.frame;

   if (UIInterfaceOrientationIsPortrait(orientation)) {
      padFrame.size.width = 500;
      padFrame.size.height = 500;
      
      padFrame.origin.x = 20.f;
      padFrame.origin.y = 480.f - padFrame.size.height / 2;
   } else {
      padFrame.size.width = 600;
      padFrame.size.height = 600;
      
      padFrame.origin.x = 100.f;
      padFrame.origin.y = 352.f - padFrame.size.height / 2;
   }
   
   padView.frame = padFrame;
   //pad sizes changed, to have correct picture,
   //I have to redraw pad's contents.
   //It seems to be fast even in case of animation,
   //but may require changes in future.
   [padView setNeedsDisplay];
}

//_________________________________________________________________
- (CGRect)centeredFrameForScrollView:(UIScrollView *)scroll andUIView:(UIView *)rView 
{
   CGSize boundsSize = scroll.bounds.size;
   CGRect frameToCenter = rView.frame;
   // center horizontally
   if (frameToCenter.size.width < boundsSize.width) {
      frameToCenter.origin.x = (boundsSize.width - frameToCenter.size.width) / 2;
   }
   else {
      frameToCenter.origin.x = 0;
   }
   // center vertically
   if (frameToCenter.size.height < boundsSize.height) {
      frameToCenter.origin.y = (boundsSize.height - frameToCenter.size.height) / 2;
   }
   else {
      frameToCenter.origin.y = 0;
   }
   
   return frameToCenter;
}

//____________________________________________________________________________________________________
- (void) centerPadFrameNoEditor : (UIInterfaceOrientation) orientation
{
   //This method only centers the pad, but does not change it's sizes.
   padView.frame = [self centeredFrameForScrollView : scrollView andUIView : padView]; 
}

//____________________________________________________________________________________________________
- (void) centerPadFrameWithEditor : (UIInterfaceOrientation) orientation
{
   padView.frame = [self centeredFrameForScrollView : scrollView andUIView : padView]; 
}

//____________________________________________________________________________________________________
- (void) correctPadFrameForOrientation : (UIInterfaceOrientation) orientation
{
   if (!zoomed) {
      if (editorView.hidden)
         [self correctPadFrameNoEditor : orientation];
      else
         [self correctPadFrameWithEditor : orientation];
   } else {
      if (editorView.hidden)
         [self centerPadFrameNoEditor : orientation];
      else
         [self centerPadFrameWithEditor : orientation];
   }
}

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


   const CGRect editorFrame = CGRectMake(mainFrame.size.width - [EditorView editorWidth], 100, [EditorView editorWidth], mainFrame.size.height - 200);
   editorView.frame = editorFrame;
   [editorView correctFrames];
   
   //Now correct padView.
   [self correctPadFrameForOrientation : orientation];
}


//____________________________________________________________________________________________________
- (id)initWithNibName:(NSString *)nibNameOrNil bundle:(NSBundle *)nibBundleOrNil
{
   self = [super initWithNibName:nibNameOrNil bundle:nibBundleOrNil];
   
   [self view];
   
   if (self) {
      self.navigationItem.rightBarButtonItem = [[UIBarButtonItem alloc] initWithTitle:@"Edit" style:UIBarButtonItemStyleBordered target:self action:@selector(showEditor)];

   
      editorView = [[EditorView alloc] initWithFrame:CGRectMake(0.f, 0.f, [EditorView editorWidth], [EditorView editorHeight])];
      grid = [[PadGridEditor alloc] initWithNibName:@"PadGridEditor" bundle:nil];
      log = [[PadLogEditor alloc] initWithNibName:@"PadLogEditor" bundle:nil];
      fill = [[FillEditor alloc] initWithNibName:@"FillEditor" bundle:nil];

      [editorView addSubEditor:fill.view withName:@"Fill"];
      [editorView addSubEditor:grid.view withName:@"Ticks and grid"];
      [editorView addSubEditor:log.view withName:@"Log scales"];
      [self.view addSubview : editorView];
      //
      scrollView.delegate = self;
      [scrollView setMaximumZoomScale:2.];
      scrollView.bounces = NO;
      //
      editorView.hidden = YES;
      [editorView release];
      //
      //Create padView, pad.
      //
      const CGPoint padCenter = CGPointMake(scrollView.frame.size.width / 2, scrollView.frame.size.height / 2);
      const CGRect padRect = CGRectMake(padCenter.x - 300.f, padCenter.y - 300.f, 600.f, 600.f);
      pad = new ROOT_iOS::Pad(600., 600.);
      padView = [[PadView alloc] initWithFrame : padRect controller : self forPad : pad];
      [scrollView addSubview : padView];
      [padView release];

      [self correctFramesForOrientation : self.interfaceOrientation];
   }
   
   return self;
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
- (void)willAnimateRotationToInterfaceOrientation : (UIInterfaceOrientation)interfaceOrientation duration : (NSTimeInterval) duration
{
   [self correctFramesForOrientation : interfaceOrientation];
}

//____________________________________________________________________________________________________
- (void) viewWillAppear:(BOOL)animated
{
   [self correctFramesForOrientation : self.interfaceOrientation];
}

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
- (void) showEditor
{
   editorView.hidden = !editorView.hidden;

   [self correctPadFrameForOrientation : self.interfaceOrientation];
   
   if (!editorView.hidden) {
      [padView turnOnEditMode];
      scrollView.editMode = YES;
   } else {
      [padView turnOffEditoMode];
      scrollView.editMode = NO;

      //If editor desappears and orientation is portrait, the
      //padView sizes has changed, the picture must be updated.
      if (UIInterfaceOrientationIsPortrait(self.interfaceOrientation))
         [padView setNeedsDisplay];
   }
   
   //Do animation.
   // First create a CATransition object to describe the transition
   CATransition *transition = [CATransition animation];
   // Animate over 3/4 of a second
   transition.duration = 0.15;
   // using the ease in/out timing function
   transition.timingFunction = [CAMediaTimingFunction functionWithName:kCAMediaTimingFunctionEaseInEaseOut];
   // Now to set the type of transition.
   transition.type = kCATransitionPush;
   
   if (!editorView.hidden)
      transition.subtype = kCATransitionFromRight;
   else
      transition.subtype = kCATransitionFromLeft;
   transition.delegate = self;
   // Next add it to the containerView's layer. This will perform the transition based on how we change its contents.
   [editorView.layer addAnimation : transition forKey : nil];
}

//____________________________________________________________________________________________________
- (void) setObject : (ObjectShortcut *)shortcut
{
   rootObject = shortcut.rootObject;

   pad->cd();
   pad->Clear();
   rootObject->Draw([shortcut.drawOption cStringUsingEncoding : [NSString defaultCStringEncoding]]);//Preserve option!!!
   [padView setNeedsDisplay];
}

#pragma mark - delegate for scroll-view.
//_________________________________________________________________
- (UIView *)viewForZoomingInScrollView:(UIScrollView *)scrollView
{
   return padView;
}

//_________________________________________________________________
- (void)scrollViewDidZoom:(UIScrollView *)scroll
{
   padView.frame = [self centeredFrameForScrollView : scroll andUIView:padView];
}

//_________________________________________________________________
- (void)scrollViewDidEndZooming:(UIScrollView *)scroll withView:(UIView *)view atScale:(float)scale
{
   const CGPoint offset = [scroll contentOffset];
   const CGRect newFrame = padView.frame;
  
   [scroll setZoomScale:1.f];
   scroll.contentSize = newFrame.size;
   scroll.contentOffset = offset;

   scroll.minimumZoomScale = 600.f / newFrame.size.width;
   scroll.maximumZoomScale = 1280.f / newFrame.size.width;

   padView.frame = newFrame;

   [padView setNeedsDisplay];
   
   zoomed = YES;
}

//_________________________________________________________________
- (void) adjustPadView
{
   zoomed = !zoomed;

   if (zoomed) {
      //maximize
   } else {

      [padView retain];
      [padView removeFromSuperview];
      padView.frame = CGRectMake(0.f, 0.f, 600.f, 600.f);
      [scrollView setZoomScale:1.f];
      scrollView.maximumZoomScale = 2.f;
      scrollView.minimumZoomScale = 1.f;
      scrollView.contentOffset = CGPointZero;
      [scrollView addSubview : padView];
      [padView release];
      [padView setNeedsDisplay];
      [self correctFramesForOrientation : self.interfaceOrientation];
   }
   
   
}

@end
