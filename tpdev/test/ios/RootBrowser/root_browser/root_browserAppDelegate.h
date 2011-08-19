//
//  root_browserAppDelegate.h
//  root_browser
//
//  Created by Timur Pocheptsov on 8/19/11.
//  Copyright 2011 __MyCompanyName__. All rights reserved.
//

#import <UIKit/UIKit.h>

@class RootFileController;

@interface root_browserAppDelegate : NSObject <UIApplicationDelegate> {

   RootFileController *rootController;
   UINavigationController *navigationController;
}

@property (nonatomic, retain) IBOutlet UIWindow *window;

@end
