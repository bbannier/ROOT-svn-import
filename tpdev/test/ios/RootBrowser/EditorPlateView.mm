//
//  EditorPlateView.m
//  editors
//
//  Created by Timur Pocheptsov on 8/16/11.
//  Copyright 2011 __MyCompanyName__. All rights reserved.
//
#import <math.h>

#import <QuartzCore/QuartzCore.h>

#import "EditorPlateView.h"
#import "EditorView.h"


@implementation EditorPlateView

@synthesize editorName = editorName_;
@synthesize arrowImageView;

//_________________________________________________________________
+ (CGFloat) plateHeight
{
   return 50.f;
}

//_________________________________________________________________
- (id)initWithFrame:(CGRect)frame editorName : (NSString *)name topView : (EditorView *) tv
{
   self = [super initWithFrame:frame];

   if (self) {
      editorLabel = [[UILabel alloc] initWithFrame:CGRectMake(frame.size.width / 2 - 60.f, 10.f, 120.f, 30.f)];
      editorLabel.backgroundColor = [UIColor clearColor];
      editorLabel.textAlignment = UITextAlignmentCenter;
      editorLabel.textColor = [UIColor darkTextColor];
      [self addSubview:editorLabel];
      [editorLabel release];
      editorLabel.text = name;
      topView = tv;

      plateImage = [UIImage imageNamed:@"editor_plate.png"];
      [plateImage retain];
        
      arrowImage = [UIImage imageNamed:@"editor_state_arrow.png"];
      [arrowImage retain];
      arrowImageView = [[UIImageView alloc] initWithImage : arrowImage];
      CGRect arrowFrame = arrowImageView.frame;
      arrowFrame.origin.x = frame.size.height / 2 - arrowFrame.size.width / 2 + 3;
      arrowFrame.origin.y = frame.size.height / 2 - arrowFrame.size.height / 2;
      arrowImageView.frame = arrowFrame;
      [self addSubview : arrowImageView];
      
      UITapGestureRecognizer *tap = [[UITapGestureRecognizer alloc] initWithTarget : self action : @selector(handleTap:)];
      [self addGestureRecognizer:tap];
      [tap release];
      self.editorName = name;
      self.opaque = NO;


//      arrowImageView.transform = CGAffineTransformMakeRotation(M_PI / 2);
   }

   return self;
}

//_________________________________________________________________
- (void)drawRect : (CGRect)rect
{
   [plateImage drawAtPoint : CGPointZero];
   //Draw the triangle.
}

//_________________________________________________________________
- (void)dealloc
{
   [arrowImage release];
   self.editorName = nil;
   [plateImage release];
   [super dealloc];
}

//_________________________________________________________________
- (void) handleTap : (UITapGestureRecognizer *) tap
{
   [topView plateTapped : self];
}

@end
