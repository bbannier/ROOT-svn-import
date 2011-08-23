//
//  FileContentController.h
//  root_browser
//
//  Created by Timur Pocheptsov on 8/19/11.
//  Copyright 2011 __MyCompanyName__. All rights reserved.
//

#import <UIKit/UIKit.h>

namespace ROOT_iOS {

class FileContainer;

}

class TObject;

@class ROOTObjectController;
@class SlideshowController;
@class ObjectShortcut;

@interface FileContentController : UIViewController {
   SlideshowController *slideshowController;
   ROOTObjectController *objectController;
   
   NSMutableArray *objectShortcuts;
   
   ROOT_iOS::FileContainer *fileContainer;
   
   IBOutlet UIScrollView *scrollView;
}

@property (nonatomic, retain) UIScrollView *scrollView;

- (void) activateForFile : (ROOT_iOS::FileContainer *)container;
- (void) selectObjectFromFile : (TObject *)obj;

@end
