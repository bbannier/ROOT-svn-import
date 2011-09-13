//
//  SlideshowController.h
//  root_browser
//
//  Created by Timur Pocheptsov on 8/19/11.
//  Copyright 2011 __MyCompanyName__. All rights reserved.
//

#import <UIKit/UIKit.h>

namespace ROOT_iOS {

class FileContainer;
class Pad;

}

@class SlideView;

@interface SlideshowController : UIViewController {
   ROOT_iOS::Pad *pads[2];
   SlideView *slides[2];

   unsigned visiblePad;
   unsigned nCurrentObject;
   
   ROOT_iOS::FileContainer *fileContainer;
}

@end
