#import "HorizontalPickerView.h"

@implementation HorizontalPickerView

const CGFloat pickerWidth = 200.f;
const CGFloat cellWidth = 50.f;
const CGFloat cellHeight = 50.f;
const CGFloat xPad = 1.5 * cellWidth;
const CGFloat markerPos = 100.f;

//____________________________________________________________________________________________________
- (id) initWithFrame : (CGRect)frame
{
   self = [super initWithFrame : frame];

   if (self) {
      self.backgroundColor = [UIColor clearColor];
   
      contentScroll = [[UIScrollView alloc] initWithFrame : CGRectMake(10.f, 10.f, pickerWidth, cellHeight)];
      contentScroll.scrollEnabled = YES;
      contentScroll.pagingEnabled = NO;
      contentScroll.delegate = self;
      contentScroll.showsVerticalScrollIndicator = NO;
      contentScroll.showsHorizontalScrollIndicator = NO;
      
      contentScroll.backgroundColor = [UIColor clearColor];
      [self addSubview : contentScroll];
      [contentScroll release];
      
      backgroundImage = [UIImage imageNamed:@"picker_bkn.png"];
      [backgroundImage retain];
      
      frameImage = [UIImage imageNamed:@"picker_frame_bkn.png"];
      [frameImage retain];

      arrowView = [[UIImageView alloc] initWithImage:[UIImage imageNamed:@"picker_arrow.png"]];
      arrowView.center = CGPointMake(frame.size.width / 2, 60 - arrowView.frame.size.height / 2);
      [self addSubview : arrowView];
      [arrowView release];
      [self bringSubviewToFront : arrowView];
   }

   return self;
}

//____________________________________________________________________________________________________
- (void) dealloc
{
   [backgroundImage release];
   [frameImage release];

   [super dealloc];
}

//____________________________________________________________________________________________________
- (void)drawRect:(CGRect)rect
{
   [frameImage drawInRect : rect];
   [backgroundImage drawInRect:CGRectMake(10.f, 10.f, 200.f, 50.f)];
}

//____________________________________________________________________________________________________
- (void) adjustScroll
{
   CGPoint offset = contentScroll.contentOffset;
   const CGFloat currentPos = markerPos + offset.x - xPad;
   const CGFloat newPos = unsigned(currentPos / cellWidth) * cellWidth + 0.5 * cellWidth;
   const CGFloat add = newPos - currentPos;
   offset.x += add;
   [contentScroll setContentOffset : offset animated : YES];
}

//____________________________________________________________________________________________________
- (void) scrollViewDidEndDecelerating : (UIScrollView *) sender
{
   [self adjustScroll];
}

- (void)scrollViewDidEndDragging:(UIScrollView *)scrollView willDecelerate:(BOOL)decelerate
{
   [self adjustScroll];
}

#pragma mark - Picker's content management.

//____________________________________________________________________________________________________
- (void) addItems : (NSMutableArray *)items
{
   NSEnumerator *enumerator = [items objectEnumerator];
   UIView *v = [enumerator nextObject];
   for (unsigned i = 0; v; v = [enumerator nextObject], ++i) {
      //Adjust view position inside a scroll:
      const CGRect viewFrame = CGRectMake(i * cellWidth + xPad, 0.f, cellWidth, cellHeight);
      v.frame = viewFrame;
      [contentScroll addSubview : v];
   }

   contentScroll.contentSize = CGSizeMake(2 * xPad + [items count] * cellWidth, cellHeight);
}

@end
