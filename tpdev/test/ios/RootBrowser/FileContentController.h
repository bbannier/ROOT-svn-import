//
//  FileContentController.h
//  root_browser
//
//  Created by Timur Pocheptsov on 8/19/11.
//  Copyright 2011 __MyCompanyName__. All rights reserved.
//

#import <UIKit/UIKit.h>

@class ROOTObjectController;
@class SlideshowController;
@class ObjectShortcut;

@interface FileContentController : UIViewController {
   NSString *fileName;
   SlideshowController *slideshowController;
   ROOTObjectController *objectController;
}

@property (nonatomic, retain) NSString *fileName;

- (void) activateForFile : (NSString *)name;
- (void) selectFromFile : (ObjectShortcut *)obj;

@end
