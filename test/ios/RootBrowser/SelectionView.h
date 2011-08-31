//
//  SelectionView.h
//  root_browser
//
//  Created by Timur Pocheptsov on 8/31/11.
//  Copyright 2011 __MyCompanyName__. All rights reserved.
//

#import <UIKit/UIKit.h>

namespace ROOT_iOS {

class Pad;

}

@interface SelectionView : UIView {
   ROOT_iOS::Pad *pad;
}

- (id)initWithFrame : (CGRect)frame withPad : (ROOT_iOS::Pad *) p;

@end
