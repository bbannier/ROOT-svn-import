//
//  ROOTObjectController.h
//  root_browser
//
//  Created by Timur Pocheptsov on 8/19/11.
//  Copyright 2011 __MyCompanyName__. All rights reserved.
//

#import <UIKit/UIKit.h>

@class PadGridEditor;
@class PadLogEditor;
@class EditorView;
@class FillEditor;

@interface ROOTObjectController : UIViewController {
   EditorView *editorView;
   
   PadGridEditor *grid;
   PadLogEditor *log;
   FillEditor *fill;
   
   IBOutlet UIScrollView *scrollView;
}

@property (nonatomic, retain) UIScrollView *scrollView;

@end
