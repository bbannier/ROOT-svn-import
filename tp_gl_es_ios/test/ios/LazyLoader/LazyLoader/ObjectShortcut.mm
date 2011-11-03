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
   using namespace ROOT::iOS::Browser;
   
   type = objType;
   if (type == ObjectType::h1d)
      thumbnail = [UIImage imageNamed : @"generic_th1.png"];
   else
      thumbnail = [UIImage imageNamed : @"generic_th2.png"];
}

- (void) drawRect : (CGRect)rect
{
   [thumbnail drawAtPoint : CGPointZero];

   CGContextRef ctx = UIGraphicsGetCurrentContext();
   CGContextSetRGBFillColor(ctx, 1.f, 1.f, 1.f, 1.f);
   const CGRect textRect = CGRectMake(0.f, [ObjectShortcut imageHeight], [ObjectShortcut imageWidth], [ObjectShortcut textHeight]);
   [objectName drawInRect : textRect withFont : [UIFont systemFontOfSize : 16] lineBreakMode : UILineBreakModeWordWrap alignment : UITextAlignmentCenter];
}


@end
