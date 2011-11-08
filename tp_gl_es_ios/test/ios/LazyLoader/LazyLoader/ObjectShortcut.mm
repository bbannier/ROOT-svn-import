#import <CoreGraphics/CGContext.h>

#import "ObjectShortcut.h"


//C++ imports.
#import "FileUtils.h"

@implementation ObjectShortcut {
   ROOT::iOS::Browser::ObjectType type;
   UIImage *thumbnail;
}

@synthesize viewTag;
@synthesize objectName;
@synthesize image = thumbnail;

+ (CGFloat) imageWidth
{
   return 150.f;
}

+ (CGFloat) imageHeight
{
   return 150.f;
}

+ (CGFloat) textHeight
{
   return 100.f;
}

+ (CGSize) shortcutSize
{
   return CGSizeMake([ObjectShortcut imageWidth], [ObjectShortcut imageHeight]);
}

- (id) initWithFrame : (CGRect)rect
{
   if (self = [super initWithFrame : rect]) {
      self.opaque = NO;
      ///
   }
   
   return self;
}

- (void) setObjectType : (ROOT::iOS::Browser::ObjectType)objType
{

   
   type = objType;
}

- (void) drawRect : (CGRect)rect
{
   using namespace ROOT::iOS::Browser;

   if (!thumbnail) {
      if (type == ObjectType::h1d)
         thumbnail = [UIImage imageNamed : @"generic_th1.png"];
      else
         thumbnail = [UIImage imageNamed : @"generic_th2.png"];
   }


   [thumbnail drawAtPoint : CGPointZero];

   CGContextRef ctx = UIGraphicsGetCurrentContext();
   CGContextSetRGBFillColor(ctx, 1.f, 1.f, 1.f, 1.f);
   const CGRect textRect = CGRectMake(0.f, [ObjectShortcut imageHeight], [ObjectShortcut imageWidth], 30.f);
   [objectName drawInRect : textRect withFont : [UIFont systemFontOfSize : 16] lineBreakMode : UILineBreakModeWordWrap alignment : UITextAlignmentCenter];
//   NSLog(@"obj name is %@", objectName);
}


@end
