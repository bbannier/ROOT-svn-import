#import <CoreGraphics/CGContext.h>
#import <QuartzCore/QuartzCore.h>

#import "RootFileController.h"
#import "FileShortcut.h"


@implementation FileShortcut

@synthesize fileName;

+ (CGFloat) fileIconWidth
{
   return 150.f;
}

+ (CGFloat) textHeight
{
   return 50.f;
}

+ (CGFloat) fileIconHeight
{
   return [FileShortcut fileIconWidth] + [FileShortcut textHeight];
}

- (id)initWithFrame:(CGRect)frame controller : (UIViewController *)c fileName : (NSString *)name contents : (unsigned int)n
{
   self = [super initWithFrame : frame];
   
   if (self) {
      controller = c;
   
      self.fileName = name;

      filePictogram = [UIImage imageNamed : @"file_icon.png"];
      [filePictogram retain];
      
      backgroundImage = [UIImage imageNamed:@"file_shortcut_background.png"];
      [backgroundImage retain];

      self.opaque = NO;
      
      UITapGestureRecognizer *tap = [[UITapGestureRecognizer alloc] initWithTarget:self action:@selector(handleTap)];
      [self addGestureRecognizer:tap];
      [tap release];
   }
   
   return self;
}

- (void)drawRect:(CGRect)rect
{
   // Drawing code
   CGContextRef ctx = UIGraphicsGetCurrentContext();

   [backgroundImage drawAtPoint:CGPointZero];

   //Draw the pictogram for ROOT's file.
   const CGPoint topLeftPicCorner = CGPointMake(rect.size.width / 2 - filePictogram.size.width / 2, 
                                                (rect.size.height - [FileShortcut textHeight]) / 2 - filePictogram.size.height / 2);
   [filePictogram drawAtPoint:topLeftPicCorner];
   
   //Draw the file name.
   CGContextSetRGBFillColor(ctx, 1.f, 1.f, 1.f, 1.f);
   const CGRect textRect = CGRectMake(0.f, [FileShortcut fileIconHeight] - [FileShortcut textHeight], [FileShortcut fileIconWidth], [FileShortcut textHeight]);
   [fileName drawInRect : textRect withFont : [UIFont systemFontOfSize : 16] lineBreakMode : UILineBreakModeWordWrap alignment : UITextAlignmentCenter];
}

- (void)dealloc
{
   self.fileName = nil;
   [filePictogram release];
   [backgroundImage release];
   [super dealloc];
}

- (void) handleTap 
{
   RootFileController *parentController = (RootFileController *)controller;
   [parentController fileWasSelected : self];
}

@end
