#import <stdlib.h>

#import <QuartzCore/QuartzCore.h>

#import "ScrollViewWithPadView.h"
#import "ROOTObjectController.h"
#import "PadImageScrollView.h"
#import "ObjectInspector.h"
#import "ObjectShortcut.h"
#import "SelectionView.h"
#import "EditorView.h"
#import "Constants.h"
#import "PadView.h"

//C++ (ROOT) imports.
#import "IOSFileContainer.h"
#import "TObject.h"
#import "TClass.h"
#import "IOSPad.h"
//#import "TFile.h"

//This constant is used to check, if pad was
//scaled to possible maximum or still can be zoomed in.
static const CGFloat scaledToMaxEpsilon = 5.f;
static const CGFloat maximumZoom = 2.f;

@implementation ROOTObjectController

@synthesize navigationScrollView;
@synthesize scrollView;

//____________________________________________________________________________________________________
- (void) initToolbarItems
{
   UIToolbar *toolbar = [[UIToolbar alloc] initWithFrame : CGRectMake(0.f, 0.f, 180.f, 44.f)];
   toolbar.barStyle = UIBarStyleBlackTranslucent;


   NSMutableArray *buttons = [[NSMutableArray alloc] initWithCapacity : 2];
   
   UIBarButtonItem *saveBtn = [[UIBarButtonItem alloc] initWithTitle:@"Save and send" style : UIBarButtonItemStyleBordered target : self action : @selector(sendEmail)];
   [buttons addObject : saveBtn];
   [saveBtn release];
   
   editBtn = [[UIBarButtonItem alloc] initWithTitle:@"Edit" style : UIBarButtonItemStyleBordered target:self action:@selector(toggleEditor)];
   [buttons addObject : editBtn];
   [editBtn release];
   
   [toolbar setItems : buttons animated : NO];
   [buttons release];
   
   UIBarButtonItem *rightItem = [[UIBarButtonItem alloc] initWithCustomView : toolbar];
   rightItem.style = UIBarButtonItemStylePlain;
   self.navigationItem.rightBarButtonItem = rightItem;
   [rightItem release];
   [toolbar release];
}

//____________________________________________________________________________________________________
- (void) viewDidLoad 
{
   [self initToolbarItems];
   [super viewDidLoad];
}

//____________________________________________________________________________________________________
- (void) resetEditorButton
{
   editBtn.style = mode == ROOT_IOSObjectController::ocmEdit ? UIBarButtonItemStyleDone : UIBarButtonItemStyleBordered;
   editBtn.title = mode == ROOT_IOSObjectController::ocmEdit ? @"Done" : @"Edit";
}

#pragma mark - Initialization code, called from initWithNibname

//____________________________________________________________________________________________________
- (void) loadObjectInspector
{
   objectInspector = [[ObjectInspector alloc] initWithNibName : nil bundle : nil];
   editorView = [objectInspector getEditorView];
   [self.view addSubview : editorView];
   editorView.hidden = YES;
}

//____________________________________________________________________________________________________
- (void) setupEditablePadView 
{
   scrollView.delegate = self;
   [scrollView setMaximumZoomScale : 2.];
   scrollView.bounces = NO;
   scrollView.bouncesZoom = NO;
   //By default, this view is hidden (mode != ocmEdit).
   scrollView.hidden = YES;
}

//____________________________________________________________________________________________________
- (void) setupNavigationScrollView 
{
   navigationScrollView.delegate = self;
   
   navigationScrollView.canCancelContentTouches = NO;
   navigationScrollView.delaysContentTouches = NO;
   navigationScrollView.bounces = NO;
   navigationScrollView.bouncesZoom = NO;
   navigationScrollView.pagingEnabled = YES;
   navigationScrollView.showsVerticalScrollIndicator = NO;
   navigationScrollView.showsHorizontalScrollIndicator = NO;
   //By default, this view is visible (mode == ocmNavigation).
   navigationScrollView.hidden = NO;
}

//____________________________________________________________________________________________________
- (void) createEditablePad
{
   using namespace ROOT_IOSBrowser;
   
   const CGPoint padCenter = CGPointMake(scrollView.frame.size.width / 2, scrollView.frame.size.height / 2);
   const CGRect padRect = CGRectMake(padCenter.x - padW / 2, padCenter.y - padH / 2, padW, padH);
   pad = new ROOT_iOS::Pad(padW, padH);
   selectedObject = pad;
   //Init the inspector for the IOSPad.
   [self setupObjectInspector];
      
   padView = [[PadView alloc] initWithFrame : padRect controller : self forPad : pad];
   [scrollView addSubview : padView];
   [padView release];
}

#pragma mark - Geometry code.

//____________________________________________________________________________________________________
- (void) correctEditablePadFrame : (UIInterfaceOrientation) orientation
{
   //The most tricky part, since this code can be called
   //for animation.
   using namespace ROOT_IOSBrowser;
   
   CGRect padFrame = CGRectMake(0.f, 0.f, padW, padH);

   if (UIInterfaceOrientationIsPortrait(orientation)) {
      padFrame.size.width = padWSmall;
      padFrame.size.height = padHSmall;
      
      padFrame.origin.x = padXWithEditorP;
      padFrame.origin.y = padYWithEditorP;
   } else {
      padFrame.origin.x = padXWithEditorL;
      padFrame.origin.y = padYWithEditorL;
   }
   
   padView.frame = padFrame;
   //pad sizes changed, to have correct picture,
   //I have to redraw pad's contents.
   //It seems to be fast even in case of animation,
   //but may require changes in future.
   [padView setNeedsDisplay];
}

//_________________________________________________________________
- (CGRect) centeredFrameForScrollView : (UIScrollView *)scroll andUIView : (UIView *)rView 
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
- (void) correctEditablePadFrameForOrientation : (UIInterfaceOrientation) orientation
{
   if (!zoomed) {
      [self correctEditablePadFrame : orientation];
   } else {
      padView.frame = [self centeredFrameForScrollView : scrollView andUIView : padView]; 
   }
}

//____________________________________________________________________________________________________
- (void) correctFramesForOrientation : (UIInterfaceOrientation) orientation
{
   using namespace ROOT_IOSBrowser;

   CGRect mainFrame;
   CGRect scrollFrame;

   if (UIInterfaceOrientationIsPortrait(orientation)) {
      mainFrame = CGRectMake(viewX, viewY, viewWP, viewHP);
      scrollFrame = CGRectMake(scrollX, scrollY, scrollWP, scrollHP);
   } else {
      mainFrame = CGRectMake(viewX, viewY, viewWL, viewHL);
      scrollFrame = CGRectMake(scrollX, scrollY, scrollWL, scrollHL);
   }
   
   self.view.frame = mainFrame;
   self.scrollView.frame = scrollFrame;
   self.navigationScrollView.frame = scrollFrame;
   
   scrollFrame.origin = CGPointZero;
   for (unsigned i = 0; i < 3; ++i) {
      scrollFrame.origin.x = i * scrollFrame.size.width;
      [navScrolls[i] resetToFrame : scrollFrame];
   }
   scrollFrame.origin = CGPointZero;
   
   if (fileContainer && fileContainer->GetNumberOfObjects() > 1) {
      navigationScrollView.contentSize = CGSizeMake(3 * scrollFrame.size.width, scrollFrame.size.height);
      [navigationScrollView scrollRectToVisible : navScrolls[1].frame animated : NO];
   } else {
      navigationScrollView.contentSize = scrollFrame.size;
   }

   const CGFloat editorAddY = 100.f;
   const CGRect editorFrame = CGRectMake(mainFrame.size.width - [EditorView editorWidth], editorAddY, [EditorView editorWidth], mainFrame.size.height - 2 * editorAddY);
   editorView.frame = editorFrame;
   [editorView correctFrames];
   
   [self correctEditablePadFrameForOrientation : orientation];
}

#pragma mark - Controller's lifecycle.

//____________________________________________________________________________________________________
- (id)initWithNibName : (NSString *)nibNameOrNil bundle : (NSBundle *)nibBundleOrNil
{
   self = [super initWithNibName : nibNameOrNil bundle : nibBundleOrNil];
   
   if (self) {
      [self view];//force view loading.
      
      mode = ROOT_IOSObjectController::ocmNavigation;

      [self loadObjectInspector];
      [self setupEditablePadView];
      [self setupNavigationScrollView];
      [self createEditablePad];
   }
   
   return self;
}

//____________________________________________________________________________________________________
- (void)dealloc
{
   self.scrollView = nil;
   self.navigationScrollView = nil;
   [objectInspector release];
 
   [super dealloc];
   
   delete pad;
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
/*
- (void) didRotateFromInterfaceOrientation:(UIInterfaceOrientation)fromInterfaceOrientation
{
   //if (mode == ROOT_IOSObjectController::ocmNavigation)
     // [self correctFramesForOrientation : self.interfaceOrientation];
}
*/

//____________________________________________________________________________________________________
- (void) viewWillAppear : (BOOL)animated
{
   [self correctFramesForOrientation : self.interfaceOrientation];
}

//____________________________________________________________________________________________________
/*
- (void)viewDidLoad
{
   [super viewDidLoad];
   // Do any additional setup after loading the view from its nib.
}
*/

//____________________________________________________________________________________________________
- (void)viewDidUnload
{
   [super viewDidUnload];
   // Release any retained subviews of the main view.
   // e.g. self.myOutlet = nil;
}

//____________________________________________________________________________________________________
- (BOOL)shouldAutorotateToInterfaceOrientation : (UIInterfaceOrientation)interfaceOrientation
{
    // Return YES for supported orientations
	return YES;
}

//____________________________________________________________________________________________________
- (void) animateEditor
{
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
- (void) resetEditablePad
{
   //Reset the pad sizes, reset the scroll, hide the editor.
   using namespace ROOT_IOSBrowser;
   
   zoomed = NO;
   padView.transform = CGAffineTransformIdentity;
   padView.frame = CGRectMake(0.f, 0.f, padW, padH);

   scrollView.contentOffset = CGPointZero;
   scrollView.maximumZoomScale = maximumZoom;
   scrollView.minimumZoomScale = 1.f;
   
   delete pad;
   pad = new ROOT_iOS::Pad(ROOT_IOSBrowser::padW, ROOT_IOSBrowser::padH);
   [padView setPad : pad];
}

//____________________________________________________________________________________________________
- (void) resetSelectionView
{
   using namespace ROOT_IOSBrowser;
   
   padView.selectionView.hidden = YES;
   padView.selectionView.transform = CGAffineTransformIdentity;
   padView.selectionView.frame = CGRectMake(0.f, 0.f, padW, padH);
}

//____________________________________________________________________________________________________
- (void) toggleEditor
{
   using namespace ROOT_IOSObjectController;
   
   mode = mode == ocmEdit ? ocmNavigation : ocmEdit;
   [self resetEditorButton];

   if (mode == ocmEdit) {
      [self resetEditablePad];
      [self resetSelectionView];
      
      selectedObject = pad;
      [self setupObjectInspector];

      pad->cd();
      pad->Clear();
      fileContainer->GetObject(currentObject)->Draw(fileContainer->GetDrawOption(currentObject));
      [padView setNeedsDisplay];

      editorView.hidden = NO;      
      navigationScrollView.hidden = YES;
      scrollView.hidden = NO;
   } else {
      pad->Unpick();
      selectedObject = pad;
      scrollView.hidden = YES;
      editorView.hidden = YES;
      
      [navScrolls[1] setObject : fileContainer->GetObject(currentObject) drawOption : fileContainer->GetDrawOption(currentObject)];
      
      navigationScrollView.hidden = NO;
   }

   [self correctFramesForOrientation : self.interfaceOrientation];

   [self animateEditor];
}

//____________________________________________________________________________________________________
- (void) adjustPrevNextIndices
{
   nextObject = currentObject + 1 < fileContainer->GetNumberOfObjects() ? currentObject + 1 : 0;
   previousObject = currentObject ? currentObject - 1 : fileContainer->GetNumberOfObjects() - 1;
}

//____________________________________________________________________________________________________
- (void) setNavigationForObjectWithIndex : (unsigned) index fromContainer : (ROOT_iOS::FileContainer *)container;
{
   //This method is called after initWithNibName was called, so it's the second step
   //of controller's construction. The default mode is ocmNavigation, so setup navigation
   //views/pad etc.
   mode = ROOT_IOSObjectController::ocmNavigation;

   
   fileContainer = container;
   self.navigationItem.title = [NSString stringWithFormat:@"%s", fileContainer->GetObject(index)->GetName()];

   const CGSize padSize = [PadImageScrollView defaultImageFrame].size;
   navPad = new ROOT_iOS::Pad(padSize.width, padSize.height);
   
   currentObject = index;
   [self adjustPrevNextIndices];
   
   CGRect scrollFrame = navigationScrollView.frame;
   scrollFrame.origin = CGPointZero;
   navScrolls[0] = [[PadImageScrollView alloc] initWithFrame : scrollFrame andPad : navPad];
   if (fileContainer->GetNumberOfObjects() == 1)
      [navScrolls[0] setObject : fileContainer->GetObject(currentObject) drawOption : fileContainer->GetDrawOption(currentObject)];
   else
      [navScrolls[0] setObject:fileContainer->GetObject(previousObject) drawOption:fileContainer->GetDrawOption(previousObject)];

   [navigationScrollView addSubview : navScrolls[0]];
   [navScrolls[0] release];

   if (fileContainer->GetNumberOfObjects() > 1) {
      //The [1] contains the current object.
      scrollFrame.origin.x = scrollFrame.size.width;
      navScrolls[1] = [[PadImageScrollView alloc] initWithFrame : scrollFrame andPad : navPad];
      [navScrolls[1] setObject:fileContainer->GetObject(currentObject) drawOption:fileContainer->GetDrawOption(currentObject)];
      [navigationScrollView addSubview : navScrolls[1]];
      [navScrolls[1] release];
      
      //The [2] contains the next object (can be the same as previous).
      scrollFrame.origin.x = scrollFrame.size.width * 2;
      navScrolls[2] = [[PadImageScrollView alloc] initWithFrame : scrollFrame andPad : navPad];
      [navScrolls[2] setObject:fileContainer->GetObject(nextObject) drawOption:fileContainer->GetDrawOption(nextObject)];
      [navigationScrollView addSubview : navScrolls[2]];
      [navScrolls[2] release];
      
      navigationScrollView.contentSize = CGSizeMake(scrollFrame.size.width * 3, scrollFrame.size.height);
      //Visible rect is always middle scroll-view ([1]).
      [navigationScrollView scrollRectToVisible : navScrolls[1].frame animated : NO];
   } else
      navigationScrollView.contentSize = scrollFrame.size;
}

#pragma mark - delegate for editable pad's scroll-view.

//____________________________________________________________________________________________________
- (UIView *)viewForZoomingInScrollView : (UIScrollView *)scrollView
{
   //For ocmEdit mode.
   return padView;
}

//____________________________________________________________________________________________________
- (void)scrollViewDidZoom:(UIScrollView *)scroll
{
   //For ocmEdit mode.
   padView.frame = [self centeredFrameForScrollView : scroll andUIView : padView];
}

//____________________________________________________________________________________________________
- (void)scrollViewDidEndZooming:(UIScrollView *)scroll withView:(UIView *)view atScale:(float)scale
{
   //For ocmEdit mode.
   using namespace ROOT_IOSBrowser;

   const CGPoint offset = [scroll contentOffset];
   const CGRect newFrame = padView.frame;
  
   [scroll setZoomScale : 1.f];
   scroll.contentSize = newFrame.size;
   scroll.contentOffset = offset;

   scroll.minimumZoomScale = padW / newFrame.size.width;
   scroll.maximumZoomScale = 2 * padW / newFrame.size.width;

   padView.transform = CGAffineTransformIdentity;

   padView.frame = newFrame;
   padView.selectionView.frame = CGRectMake(0.f, 0.f, newFrame.size.width, newFrame.size.height);
   
   //
   pad->SetViewWH(newFrame.size.width, newFrame.size.height);
   //

   [padView setNeedsDisplay];
   
   zoomed = YES;
}

//____________________________________________________________________________________________________
- (CGRect)zoomRectForScale:(float)scale withCenter:(CGPoint)center {
    
    CGRect zoomRect;
    
    // the zoom rect is in the content view's coordinates. 
    //    At a zoom scale of 1.0, it would be the size of the imageScrollView's bounds.
    //    As the zoom scale decreases, so more content is visible, the size of the rect grows.
    zoomRect.size.height = [scrollView frame].size.height / scale;
    zoomRect.size.width  = [scrollView frame].size.width  / scale;
    
    // choose an origin so as to get the right center.
    zoomRect.origin.x    = center.x - (zoomRect.size.width  / 2.0);
    zoomRect.origin.y    = center.y - (zoomRect.size.height / 2.0);
    
    return zoomRect;
}

//____________________________________________________________________________________________________
- (void) handleDoubleTapOnPad : (CGPoint) tapPt
{
   //For ocmEdit mode.
   using namespace ROOT_IOSBrowser;

   BOOL scaleToMax = YES;
   
   if (fabs(padView.frame.size.width - padW * maximumZoom) < scaledToMaxEpsilon)
      scaleToMax = NO;

   if (scaleToMax) {
      //maximize
      zoomed = YES;
      const CGFloat newScale = padW * maximumZoom / padView.frame.size.width;
      CGRect zoomRect = [self zoomRectForScale : newScale withCenter : tapPt];
      [scrollView zoomToRect : zoomRect animated : YES];
//      [self scrollViewDidEndZooming : scrollView withView : padView atScale : maximumZoom];
   } else {
      zoomed = NO;
      padView.frame = CGRectMake(0.f, 0.f, padW, padH);
      padView.selectionView.frame = CGRectMake(0.f, 0.f, padW, padH);
      //
      pad->SetViewWH(padW, padH);
      //
      scrollView.maximumZoomScale = maximumZoom;
      scrollView.minimumZoomScale = 1.f;
      scrollView.contentOffset = CGPointZero;
      scrollView.contentSize = padView.frame.size;
      //[scrollView addSubview : padView];
      //[padView release];
      [padView setNeedsDisplay];
      [self correctFramesForOrientation : self.interfaceOrientation];
   }
}

#pragma mark - picking and editing.

//____________________________________________________________________________________________________
- (void) objectWasSelected : (TObject *)object
{
   if (object != selectedObject) {//New object was selected.
      object ? selectedObject = object : (selectedObject = pad);
      [self setupObjectInspector];
      [objectInspector resetInspector];
   }

   if (object) {
      padView.selectionView.hidden = NO;
      [padView.selectionView setNeedsDisplay];
   } else
      padView.selectionView.hidden = YES;   
}

//____________________________________________________________________________________________________
- (void) setupObjectInspector
{
   [objectInspector setROOTObject : selectedObject];
   [objectInspector setROOTObjectController : self];
}

//____________________________________________________________________________________________________
- (void) objectWasModifiedUpdateSelection : (BOOL)needUpdate
{
   if (needUpdate)
      pad->InvalidateSelection();
   
   [padView setNeedsDisplay];
}

#pragma mark - File contents navigation: scrolling through objects.

//____________________________________________________________________________________________________
- (void) scrollToLeft
{
   currentObject + 1 < fileContainer->GetNumberOfObjects() ? ++currentObject : currentObject = 0;
   [self adjustPrevNextIndices];
   //Current is becoming prev, next is becoming current, load new next.
   UIImage *prevImage = navScrolls[1].padImage;
   [prevImage retain];
   UIImage *currentImage = navScrolls[2].padImage;
   [currentImage retain];
      
   [navScrolls[0] setObject : fileContainer->GetObject(previousObject) drawOption : fileContainer->GetDrawOption(previousObject) andImage : prevImage];
   [navScrolls[1] setObject : fileContainer->GetObject(currentObject) drawOption : fileContainer->GetDrawOption(currentObject) andImage : currentImage];
   [prevImage release];
   [currentImage release];
   
   [navScrolls[2] setObject : fileContainer->GetObject(nextObject) drawOption : fileContainer->GetDrawOption(nextObject)];

   [navigationScrollView scrollRectToVisible : navScrolls[1].frame animated : NO];
   self.navigationItem.title = [NSString stringWithFormat : @"%s", fileContainer->GetObject(currentObject)->GetName()];
}

//____________________________________________________________________________________________________
- (void) scrollToRight
{
   currentObject ? --currentObject : currentObject = fileContainer->GetNumberOfObjects() - 1;
   [self adjustPrevNextIndices];
   //Current is becoming next, prev - current, prev must be loaded.
   UIImage *nextImage = navScrolls[1].padImage;
   [nextImage retain];
   UIImage *currImage = navScrolls[0].padImage;
   [currImage retain];
   
   [navScrolls[1] setObject : fileContainer->GetObject(currentObject) drawOption : fileContainer->GetDrawOption(currentObject) andImage : currImage];
   [navScrolls[2] setObject : fileContainer->GetObject(nextObject) drawOption : fileContainer->GetDrawOption(nextObject) andImage : nextImage];
   [navScrolls[0] setObject : fileContainer->GetObject(previousObject) drawOption : fileContainer->GetDrawOption(previousObject)];
   [nextImage release];
   [currImage release];
   
   [navigationScrollView scrollRectToVisible : navScrolls[1].frame animated : NO];
   self.navigationItem.title = [NSString stringWithFormat : @"%s", fileContainer->GetObject(currentObject)->GetName()];
}

//____________________________________________________________________________________________________
- (void) scrollViewDidEndDecelerating : (UIScrollView *) sender
{
   if (sender == navigationScrollView) {
      if (sender.contentOffset.x > navigationScrollView.frame.size.width)
         [self scrollToLeft];
      else if (sender.contentOffset.x < navigationScrollView.frame.size.width)
         [self scrollToRight];
   }
}

#pragma mark - Save modified object as pdf and root files.

//___________________________________________________________
- (void) createPDFFileWithPage :(CGRect)pageRect fileName : (const char*)filename
{	
	CFStringRef path = CFStringCreateWithCString (NULL, filename, kCFStringEncodingUTF8);
	CFURLRef url = CFURLCreateWithFileSystemPath (NULL, path, kCFURLPOSIXPathStyle, 0);
	CFRelease(path);
	// This dictionary contains extra options mostly for 'signing' the PDF
	CFMutableDictionaryRef myDictionary = CFDictionaryCreateMutable(NULL, 0, &kCFTypeDictionaryKeyCallBacks, &kCFTypeDictionaryValueCallBacks);

	CFDictionarySetValue(myDictionary, kCGPDFContextTitle, CFSTR("PDF File"));
	CFDictionarySetValue(myDictionary, kCGPDFContextCreator, CFSTR("Timur Pocheptsov"));
	// Create our PDF Context with the CFURL, the CGRect we provide, and the above defined dictionary
	CGContextRef ctx = CGPDFContextCreateWithURL (url, &pageRect, myDictionary);
	// Cleanup our mess
	CFRelease(myDictionary);
	CFRelease(url);
	// Done creating our PDF Context, now it's time to draw to it
	
	// Starts our first page
	CGContextBeginPage (ctx, &pageRect);	
	// Draws a black rectangle around the page inset by 50 on all sides
   CGContextSetRGBFillColor(ctx, 1.f, 0.4f, 0.f, 1.f);
   CGContextFillRect(ctx, pageRect);

   ROOT_iOS::Pad *padToSave = mode == ROOT_IOSObjectController::ocmEdit ? pad : navPad;
   padToSave->cd();
   
   if (mode == ROOT_IOSObjectController::ocmNavigation) {
      padToSave->Clear();
      fileContainer->GetObject(currentObject)->Draw(fileContainer->GetDrawOption(currentObject));
   }
   
   padToSave->cd();
   padToSave->SetContext(ctx);
   padToSave->SetViewWH(pageRect.size.width, pageRect.size.height);
   padToSave->Paint();

   CGContextEndPage(ctx);
	// We are done with our context now, so we release it
	CGContextRelease (ctx);
}

#pragma mark - MFMailComposeViewController delegate

//___________________________________________________________
- (void) sendEmail
{
	NSArray *paths = NSSearchPathForDirectoriesInDomains(NSDocumentDirectory, NSUserDomainMask, YES);
	NSString *saveDirectory = [paths objectAtIndex : 0];
   NSString *saveFileName = [NSString stringWithFormat:@"%s.pdf", fileContainer->GetObject(currentObject)->GetName()];
	NSString *newFilePath = [saveDirectory stringByAppendingPathComponent : saveFileName];
	const char *filename = [newFilePath UTF8String];
   
   [self createPDFFileWithPage: CGRectMake(0, 0, 600, 600) fileName : filename];
//
/*
   NSString *rootFileName = [NSString stringWithFormat:@"%s.root", fileContainer->GetObject(currentObject)->GetName()];
	NSString *rootFilePath = [saveDirectory stringByAppendingPathComponent : rootFileName];
   const char *cFileName = [rootFilePath UTF8String];
   TFile f(cFileName, "recreate");
   f.cd();
   fileContainer->GetObject(currentObject)->Write();
*/
//

   MFMailComposeViewController * mailComposer = [[MFMailComposeViewController alloc] init];
   [mailComposer setSubject:@"E-mail from ROOT's iPad"];
   [mailComposer setMessageBody : @"This is a test message sent to you by ROOT browser for iPad" isHTML : NO];
   mailComposer.mailComposeDelegate = self;

   NSString *path = [NSString stringWithFormat : @"%s", filename];
   if ([[NSFileManager defaultManager] fileExistsAtPath : path]) {
      NSData *myData = [NSData dataWithContentsOfFile : path];
      [mailComposer addAttachmentData : myData mimeType : @"application/octet-stream" fileName : saveFileName];
   }

   [self presentModalViewController : mailComposer animated : YES];
   [mailComposer release];
}

//___________________________________________________________
- (void) mailComposeController : (MFMailComposeViewController *)controller didFinishWithResult : (MFMailComposeResult)result error : (NSError *)error
{
   [self becomeFirstResponder];
   [self dismissModalViewControllerAnimated : YES];
}

@end
