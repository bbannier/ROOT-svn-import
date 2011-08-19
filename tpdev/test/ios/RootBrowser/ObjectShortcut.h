//
//  ObjectShortcut.h
//  root_browser
//
//  Created by Timur Pocheptsov on 8/19/11.
//  Copyright 2011 __MyCompanyName__. All rights reserved.
//

#import <UIKit/UIKit.h>

@class FileContentController;

@interface ObjectShortcut : UIView {
   UIImage *icon;
   NSString *objectName;
   
   FileContentController *controller;
}

@property (nonatomic, retain) UIImage *icon;
@property (nonatomic, retain) NSString *objectName;

+ (CGFloat) iconWidth;
+ (CGFloat) iconHeight;
+ (CGFloat) textHeight;
+ (CGRect) defaultRect;


- (id) initWithFrame : (CGRect)frame controller : (FileContentController*) c objectName : (NSString *)name;
- (void) drawRect : (CGRect)rect;

@end
