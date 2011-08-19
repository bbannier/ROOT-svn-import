//
//  RootFileController.h
//  root_browser
//
//  Created by Timur Pocheptsov on 8/19/11.
//  Copyright 2011 __MyCompanyName__. All rights reserved.
//

#import <UIKit/UIKit.h>

@class FileContentController;
@class FileShortcut;

@interface RootFileController : UIViewController <UINavigationControllerDelegate, UINavigationBarDelegate> {
   NSMutableArray *fileContainers;
   FileContentController *contentController;
}

- (void) fileWasSelected : (FileShortcut*) shortcut;
- (void) addFiles;

@end
