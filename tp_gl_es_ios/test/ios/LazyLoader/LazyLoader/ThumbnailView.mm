#import <CoreGraphics/CGContext.h>

#import "ThumbnailViewDatasource.h"
#import "ObjectShortcut.h"
#import "ThumbnailView.h"

const CGFloat addSpace = 100.f;

@interface ThumbnailView () {
   UIScrollView *scrollView;
   
   NSMutableSet *visibleThumbnails;
   NSMutableSet *cachedThumbnails;

   unsigned nThumbnails;
   unsigned nCols;
   unsigned nRows;
}

- (void) lateInit;

@end

@implementation ThumbnailView

@synthesize dataSource;

- (id)initWithFrame : (CGRect)frame
{
   if (self = [super initWithFrame:frame]) {
      [self lateInit];
   }

   return self;
}

- (void) awakeFromNib
{
   [self lateInit];
}

/*
- (void) drawRect : (CGRect)rect
{
   //Debug method to check view's placement and geometry.
   CGContextRef ctx = UIGraphicsGetCurrentContext();
   
   CGContextSetRGBFillColor(ctx, 1.f, 0.f, 0.f, 1.f);
   CGContextFillRect(ctx, rect);
   
   CGContextSetRGBStrokeColor(ctx, 0.f, 0.f, 0.f, 1.f);
   CGContextBeginPath(ctx);
   CGContextMoveToPoint(ctx, 0.f, rect.size.height / 2);
   CGContextAddLineToPoint(ctx, rect.size.width, rect.size.height / 2);
   CGContextStrokePath(ctx);
   CGContextBeginPath(ctx);
   CGContextMoveToPoint(ctx, rect.size.width / 2, 0.f);
   CGContextAddLineToPoint(ctx, rect.size.width / 2, rect.size.height);
   CGContextStrokePath(ctx);

   const CGFloat sz = 20.f;
   
   CGContextSetRGBFillColor(ctx, 0.f, 1.f, 0.f, 1.f);
   CGContextFillRect(ctx, CGRectMake(0.f, rect.size.height / 2, sz, sz));
   CGContextFillRect(ctx, CGRectMake(rect.size.width - sz, rect.size.height / 2, sz, sz));
   CGContextFillRect(ctx, CGRectMake(rect.size.width / 2, 0.f, sz, sz));
   CGContextFillRect(ctx, CGRectMake(rect.size.width / 2, rect.size.height - sz, sz, sz));
}
*/

- (void) addToCache : (UIView *)view
{
   [cachedThumbnails addObject : view];
   [view removeFromSuperview];
}

- (void) configureThumbnails
{
   
}


- (void) loadThumbnails
{
   nThumbnails = [dataSource numberOfThumbnails];

   // recycle all items
   for (UIView *view in visibleThumbnails)
      [self addToCache : view];

   [visibleThumbnails removeAllObjects];
   [self configureThumbnails];
}

- (void) lateInit
{
   scrollView = (UIScrollView *)[self viewWithTag : 5];
   
   visibleThumbnails = [[NSMutableSet alloc] init];
   cachedThumbnails = [[NSMutableSet alloc] init];
}

- (void) configureView
{
   const CGRect viewRect = self.bounds;
   
   nCols = unsigned(viewRect.size.width / ([ObjectShortcut imageWidth] + addSpace));
   nRows = (nThumbnails + nCols - 1) / nCols;

   [self configureThumbnails];
}

@end
