//
//  ObjectShortcut.h
//  root_browser
//
//  Created by Timur Pocheptsov on 8/19/11.
//  Copyright 2011 __MyCompanyName__. All rights reserved.
//

#import <UIKit/UIKit.h>

@class FileContentController;

class TObject;

@interface ObjectShortcut : UIView {
   UIImage *icon;
   
   FileContentController *controller;
   TObject *rootObject;
   NSString *drawOption;
}

@property (nonatomic, retain) UIImage *icon;
@property (nonatomic, retain) NSString *drawOption;
@property (nonatomic, assign) TObject *rootObject;

+ (CGFloat) iconWidth;
+ (CGFloat) iconHeight;
+ (CGFloat) textHeight;
+ (CGRect) defaultRect;


- (id) initWithFrame : (CGRect)frame controller : (FileContentController*) c object : (TObject *)object andOption : (const char*)option thumbnail : (UIImage *)thumbnail;
- (void) drawRect : (CGRect)rect;

@end
