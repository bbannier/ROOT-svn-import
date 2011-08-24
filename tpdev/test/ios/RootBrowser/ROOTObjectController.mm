//
//  ROOTObjectController.m
//  root_browser
//
//  Created by Timur Pocheptsov on 8/19/11.
//  Copyright 2011 __MyCompanyName__. All rights reserved.
//

#import <QuartzCore/QuartzCore.h>

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
- (void) correctFrames
{
   self.view.alpha = 0.f;

   CGRect mainFrame;
   CGRect scrollFrame;

   const UIInterfaceOrientation orientation = self.interfaceOrientation;
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
   const CGPoint padCenter = CGPointMake(scrollView.frame.size.width / 2, scrollView.frame.size.height / 2);
   const CGRect padRect = CGRectMake(padCenter.x - 300.f, padCenter.y - 300.f, 600.f, 600.f);
   padView.frame = padRect;
   
   self.view.alpha = 1.f;
}


//____________________________________________________________________________________________________
- (id)initWithNibName:(NSString *)nibNameOrNil bundle:(NSBundle *)nibBundleOrNil
{
   self = [super initWithNibName:nibNameOrNil bundle:nibBundleOrNil];
   
   [self view];
   
   if (self) {
      editorView = [[EditorView alloc] initWithFrame:CGRectMake(0.f, 0.f, [EditorView editorWidth], [EditorView editorHeight])];
      grid = [[PadGridEditor alloc] initWithNibName:@"PadGridEditor" bundle:nil];
      log = [[PadLogEditor alloc] initWithNibName:@"PadLogEditor" bundle:nil];
      fill = [[FillEditor alloc] initWithNibName:@"FillEditor" bundle:nil];

      [editorView addSubEditor:fill.view withName:@"Fill"];
      [editorView addSubEditor:grid.view withName:@"Ticks and grid"];
      [editorView addSubEditor:log.view withName:@"Log scales"];
      [self.view addSubview : editorView];
      
      UITapGestureRecognizer * tap = [[UITapGestureRecognizer alloc] initWithTarget:self action:@selector(showEditor)];
      [self.view addGestureRecognizer:tap];
      [tap release];
   
      //
      scrollView.delegate = self;
      [scrollView setMaximumZoomScale:3.];
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
      padView = [[PadView alloc] initWithFrame : padRect forPad : pad];
      [scrollView addSubview : padView];
      [padView release];

      [self correctFrames];
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
- (void) viewWillAppear:(BOOL)animated
{
   [self correctFrames];
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
- (void)willRotateToInterfaceOrientation:(UIInterfaceOrientation)toInterfaceOrientation duration:(NSTimeInterval)duration
{
   //Cheap thay to hide and not check previos "hidden" state later.
   editorView.alpha = 0.f;
   scrollView.alpha = 0.f;
}

//____________________________________________________________________________________________________
- (void)didRotateFromInterfaceOrientation:(UIInterfaceOrientation)fromInterfaceOrientation
{
   [self correctFrames];
   //hidden or not, alpha was set to 0., now return back.
   editorView.alpha = 1.f;
   scrollView.alpha = 1.f;
}

//____________________________________________________________________________________________________
- (void) showEditor
{
   editorView.hidden = !editorView.hidden;
   
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

@end
