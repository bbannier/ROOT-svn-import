#import <Foundation/Foundation.h>

//
//All editors must adopt this protocol.
//

@class ROOTObjectController;

class TObject;

@protocol ROOTObjectEditorProtocol <NSObject>

- (void) setROOTObjectController : (ROOTObjectController *)controller;
- (void) setROOTObject : (TObject *)obj;

@end
