//
//  EditorView.m
//  editors
//
//  Created by Timur Pocheptsov on 8/16/11.
//  Copyright 2011 __MyCompanyName__. All rights reserved.
//

#import <math.h>

#import <CoreGraphics/CGContext.h>

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
      const CGRect scrollFrame = CGRectMake(10.f, 10.f, [EditorView scrollWidth], [EditorView scrollHeight]);
      scrollView = [[UIScrollView alloc] initWithFrame:scrollFrame];
      scrollView.backgroundColor = [UIColor clearColor];
      scrollView.autoresizingMask = UIViewAutoresizingFlexibleLeftMargin | UIViewAutoresizingFlexibleRightMargin | UIViewAutoresizingFlexibleTopMargin | UIViewAutoresizingFlexibleBottomMargin;
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
   CGContextRef ctx = UIGraphicsGetCurrentContext();
   if (!ctx) {
      //Log error: ctx is nil.
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
- (void) setViewsYs
{
   //These are new sub-views positions after animation.
   for (unsigned i = 0; i < nEditors; ++i) {
      CGRect frame = views[i].frame;
      if (currentState & (1 << i)) {//View will become visible now.
         frame.origin.y = viewYs[currentState * nEditors + i];
         views[i].frame = frame;
      } else if (!views[i].hidden) {//View will hide now.
         const unsigned prevState = currentState | (1 << i);
         frame.origin.y = viewYs[prevState * nEditors + i] - plates[i].frame.size.height;
         views[i].frame = frame;
      }
   }
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
      //Log error: too many editors.
      return;
   }

   //Save editor.
   views[nEditors] = element;
   
   plates[nEditors] = [[EditorPlateView alloc] initWithFrame:CGRectMake(0.f, 0.f, [EditorView scrollWidth], [EditorPlateView plateHeight]) editorName : name topView : self];
   [scrollView addSubview : plates[nEditors]];

   [plates[nEditors] release];

   [scrollView addSubview : element];

   //New number of sub-editors and possible editor states.
   ++nEditors;
   nStates = 1 << nEditors;
   
   const CGFloat totalHeight = [self recalculateEditorGeometry];
   scrollView.contentSize = CGSizeMake([EditorView scrollWidth], totalHeight);
   scrollView.contentOffset = CGPointZero;
   
   currentState = 0;
   element.hidden = YES; //Initially, the sub-editor is hidden.
   
   element.alpha = 0.f;
   
   [self setPlatesYs];
   for (unsigned i = 0; i < nEditors; ++i)
      [scrollView bringSubviewToFront : plates[i]];
}

//_________________________________________________________________
- (void) presetViewsYs
{
   //These are sub-views positions before animation.
   for (unsigned i = 0; i < nEditors; ++i) {
      CGRect frame = views[i].frame;
      //If view must appear now:
      if (views[i].hidden && (currentState & (1 << i))) {
         frame.origin.y = viewYs[currentState * nEditors + i] - plates[i].frame.size.height;
         views[i].frame = frame;
      }
   }
}

//_________________________________________________________________
- (void) setViewsAlpha
{
   for (unsigned i = 0; i < nEditors; ++i) {
      EditorPlateView *p = (EditorPlateView*)plates[i];
      UIView *v = views[i];
      const BOOL nowVisible = currentState & (1 << i);
      if (v.hidden) {
         if (nowVisible) {
            v.hidden = NO;
            v.alpha = 1.f;
            p.arrowImageView.transform = CGAffineTransformMakeRotation(M_PI / 2);//rotate the arrow.
         }
      } else {
         if (!nowVisible) {
            p.arrowImageView.transform = CGAffineTransformMakeRotation(0.f);
            v.alpha = 0.f;
         }
      }
   }
}

//_________________________________________________________________
- (void) setViewsVisibility
{
   for (unsigned i = 0; i < nEditors; ++i) {
      if (!(currentState & (1 << i)))
         views[i].hidden = YES;
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
   [self setViewsAlpha];
   
   [UIView commitAnimations];
   
   [self setViewsVisibility];
}

//_________________________________________________________________
- (void) plateTapped : (EditorPlateView *) plate
{
   for (unsigned i = 0; i < nEditors; ++i) {
      if (plate == plates[i]) {
         currentState ^= (1 << i);
         [self animateEditor];
      }
   }
}

@end
