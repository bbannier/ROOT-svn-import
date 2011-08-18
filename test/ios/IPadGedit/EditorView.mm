//
//  EditorView.m
//  editors
//
//  Created by Timur Pocheptsov on 8/16/11.
//  Copyright 2011 __MyCompanyName__. All rights reserved.
//

#import <math.h>

#import <CoreGraphics/CGContext.h>

#import "ScrollViewWithPickers.h"
#import "EditorPlateView.h"
#import "EditorView.h"


@implementation EditorView

//_________________________________________________________________
+ (CGFloat) editorAlpha
{
   return 0.4f;
}

//_________________________________________________________________
+ (CGFloat) editorWidth
{
   return 270.f;
}

//_________________________________________________________________
+ (CGFloat) editorHeight
{
   return 650.f;
}

//_________________________________________________________________
+ (CGFloat) scrollWidth
{
   return [EditorView editorWidth] - 20.f;
}

//_________________________________________________________________
+ (CGFloat) scrollHeight
{
   return [EditorView editorHeight] - 20.f;
}

//_________________________________________________________________
+ (CGFloat) ncWidth
{
   return 20.f;
}

//_________________________________________________________________
+ (CGFloat) ncHeight
{
   return 20.f;
}

//_________________________________________________________________
- (id)initWithFrame : (CGRect)frame
{
   self = [super initWithFrame : frame];

   if (self) {
      //Scroll view is a container for all sub-editors.
      //It's completely transparent.
      const CGRect scrollFrame = CGRectMake(10.f, 10.f, [EditorView scrollWidth], [EditorView scrollHeight]);
      scrollView = [[ScrollViewWithPickers alloc] initWithFrame : scrollFrame];
      scrollView.backgroundColor = [UIColor clearColor];
      scrollView.autoresizingMask = UIViewAutoresizingFlexibleLeftMargin | UIViewAutoresizingFlexibleRightMargin | UIViewAutoresizingFlexibleTopMargin | UIViewAutoresizingFlexibleBottomMargin;
      scrollView.bounces = NO;
      [self addSubview : scrollView];
      [scrollView release];
      self.opaque = NO;
   }

   return self;
}

//_________________________________________________________________
- (void)dealloc
{
    [super dealloc];
}

//_________________________________________________________________
- (void)drawRect:(CGRect)rect
{
   //Draw main editor's view as a semi-transparent
   //gray view with rounded corners.

   CGContextRef ctx = UIGraphicsGetCurrentContext();
   if (!ctx) {
      NSLog(@"[EditorView drawRect:], ctx is nil");
      return;
   }
   
   UIColor *background = [[UIColor darkGrayColor] colorWithAlphaComponent : [EditorView editorAlpha]];
   CGContextSetFillColorWithColor(ctx, background.CGColor);
   //Draw the rect with rounded corners now.
   CGContextFillRect(ctx, CGRectMake(0.f, [EditorView ncHeight] / 2, [EditorView ncWidth] / 2, [EditorView editorHeight] - [EditorView ncHeight]));
   CGContextFillRect(ctx, CGRectMake([EditorView ncWidth] / 2, 0.f, [EditorView editorWidth] - [EditorView ncWidth] / 2, [EditorView editorHeight]));
   
   //Draw arcs.
   CGContextBeginPath(ctx);
   CGContextMoveToPoint(ctx, [EditorView ncWidth] / 2, [EditorView ncHeight] / 2);
   CGContextAddArc(ctx, [EditorView ncWidth] / 2, [EditorView ncHeight] / 2, [EditorView ncWidth] / 2, M_PI, 3 * M_PI / 2, 0);
   CGContextFillPath(ctx);
   //
   CGContextBeginPath(ctx);
   CGContextMoveToPoint(ctx, [EditorView ncWidth] / 2, [EditorView editorHeight] - [EditorView ncHeight] / 2);
   CGContextAddArc(ctx, [EditorView ncWidth] / 2, [EditorView editorHeight] - [EditorView ncHeight] / 2, [EditorView ncWidth] / 2, M_PI / 2, M_PI, 0);
   CGContextFillPath(ctx);

}

//_________________________________________________________________
- (void) propertyUpdated
{
}

//_________________________________________________________________
- (CGFloat) recalculateEditorGeometry
{
   const CGFloat dY = 10.f;//space between controls.
   for (unsigned i = 0; i < nStates; ++i) {
      CGFloat currentY = 0.f;
      for (unsigned j = 0; j < nEditors; ++j) {
         plateYs[nEditors * i + j] = currentY;
         currentY += plates[j].frame.size.height + dY;

         const unsigned editorBit = 1 << j;
         if (i & editorBit) {//In this state, j-th editor is visible.
            viewYs[nEditors * i + j] = currentY;
            currentY += views[j].frame.size.height + dY;
         } else
            viewYs[nEditors * i + j] = 0.f;//coordinate is not used, currentY does not need update.
      }
   }

   //Now, the total container height.
   CGFloat totalHeight = 0.f;
   for (unsigned i = 0; i < nEditors; ++i)
      totalHeight += plates[i].frame.size.height + dY + views[i].frame.size.height + dY;

   return totalHeight;
}

//_________________________________________________________________
- (void) setPlatesYs
{
   //Plates positions for the current state.
   for (unsigned i = 0; i < nEditors; ++i) {
      CGRect frame = plates[i].frame;
      frame.origin.y = plateYs[nEditors * currentState + i];
      plates[i].frame = frame;
   }
}

//_________________________________________________________________
- (void) addSubEditor:(UIView *)element withName : (NSString *)name
{
   if (nEditors == evMaxComponents) {
      NSLog(@"Could not add more editors");
      return;
   }

   //Add this new editor.
   views[nEditors] = element;
   //1. Plate with editor's name and triangle, showing the editor's state.
   //topView is 'self' - the view, which will be informed, that user tapped on editor's plate.
   plates[nEditors] = [[EditorPlateView alloc] initWithFrame : CGRectMake(0.f, 0.f, [EditorView scrollWidth], [EditorPlateView plateHeight]) editorName : name topView : self];
   [scrollView addSubview : plates[nEditors]];
   [plates[nEditors] release];

   //Create a container view for sub-editor.
   CGRect elementFrame = element.frame;
   elementFrame.origin = CGPointZero;
   containers[nEditors] = [[UIView alloc] initWithFrame : elementFrame];
   element.frame = elementFrame;
   //Place sub-editor into the container view.
   [containers[nEditors] addSubview : element];
   //Clip to bounds: when we animate sub-editor (appear/disappera)
   //it moves from/to negative coordinates and this negative part
   //should not be visible.
   containers[nEditors].clipsToBounds = YES;
   //Initially, container with sub-editor is hidden.
   containers[nEditors].hidden = YES;
   //Add container.
   [scrollView addSubview : containers[nEditors]];
   [containers[nEditors] release];

   //New number of sub-editors and possible editor states.
   ++nEditors;
   nStates = 1 << nEditors;

   //Recalculate possible positions of all plates and containers.
   const CGFloat totalHeight = [self recalculateEditorGeometry];
   //Set scrollView.contentSize to include all sub-editors in opened state.
   scrollView.contentSize = CGSizeMake([EditorView scrollWidth], totalHeight);
   scrollView.contentOffset = CGPointZero;
   
   //No sub-editor is visible.
   currentState = 0;
   //Also, make new sub-editor transparent.
   element.alpha = 0.f;

   //Place all plates.
   [self setPlatesYs];
}

//_________________________________________________________________
- (void) presetViewsYs
{
   //These are sub-views positions before animation.
   for (unsigned i = 0; i < nEditors; ++i) {
      //If view must appear now:
      if (containers[i].hidden && (currentState & (1 << i))) {
         CGRect frame = views[i].frame;
         frame.origin.y = viewYs[currentState * nEditors + i];
         //Place the container in a correct position.
         containers[i].frame = frame;
         //Contained view is shifted in a container (it will appear from nowhere).
         frame.origin.y = -frame.size.height;
         views[i].frame = frame;
      }
   }
}

//_________________________________________________________________
- (void) setViewsYs
{
   //These are new sub-views positions at the end of animation.
   for (unsigned i = 0; i < nEditors; ++i) {
      CGRect frame = views[i].frame;
      if (currentState & (1 << i)) {//View will become visible now (and could be visible before).
         frame.origin.y = viewYs[currentState * nEditors + i];
         containers[i].frame = frame;
         frame.origin.y = 0.;
         views[i].frame = frame;
      } else if (!views[i].hidden) {//View will hide now - it moves outside of container.
         frame.origin.y = -frame.size.height;
         views[i].frame = frame;
      }
   }
}

//_________________________________________________________________
- (void) setViewsAlphaAndVisibility
{
   //During animation, if view will appear it's alpha changes from 0 to 1,
   //and if it's going to disappear - from 1 to 0.
   //Also, I have to animate small triangle, which
   //shows editor's state (hidden/visible).
   for (unsigned i = 0; i < nEditors; ++i) {
      EditorPlateView *p = (EditorPlateView *)plates[i];
      UIView *v = views[i];
      const BOOL nowVisible = currentState & (1 << i);
      if (containers[i].hidden) {
         if (nowVisible) {
            containers[i].hidden = NO;
            v.alpha = 1.f;
            p.arrowImageView.transform = CGAffineTransformMakeRotation(M_PI / 2);//rotate the triangle.
         }
      } else {
         if (!nowVisible) {
            p.arrowImageView.transform = CGAffineTransformMakeRotation(0.f);//rotate the triangle.
            v.alpha = 0.f;
         }
      }
   }
}

//_________________________________________________________________
- (void) hideViews
{
   for (unsigned i = 0; i < nEditors; ++i) {
      if (!(currentState & (1 << i)))
         containers[i].hidden = YES;
   }
}

//_________________________________________________________________
- (void) animateEditor
{
   [self presetViewsYs];
   
   [UIView beginAnimations : nil context : nil];
   [UIView setAnimationDuration : 0.25];
   [UIView setAnimationCurve : UIViewAnimationCurveEaseOut];
 
   [self setPlatesYs];
   [self setViewsYs];
   [self setViewsAlphaAndVisibility];

   [UIView commitAnimations];
 
   //Do not hide the views immediately, so user can see animation.
   [NSTimer scheduledTimerWithTimeInterval : 0.25 target : self selector:@selector(hideViews) userInfo:nil repeats:NO];
}

//_________________________________________________________________
- (void) plateTapped : (EditorPlateView *) plate
{
   //User has tapped on editor's plate.
   //Depending on the current editor's state,
   //we open or close it with animation.
   for (unsigned i = 0; i < nEditors; ++i) {
      if (plate == plates[i]) {
         currentState ^= (1 << i);
         [self animateEditor];
      }
   }
}

@end
