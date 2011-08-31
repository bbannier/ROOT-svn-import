//
//  EditorRTTI.cpp
//  root_browser
//
//  Created by Timur Pocheptsov on 8/31/11.
//  Copyright 2011 __MyCompanyName__. All rights reserved.
//

#import "TAttLine.h"
#import "TAttFill.h"
#import "TObject.h"
#import "TAttPad.h"

//Other attribs later.

#import "EditorRTTI.h"

namespace ROOT_IOSBrowser {

unsigned GetRequiredEditors(const TObject *obj)
{
   unsigned editors = 0;//bit mask.
   
   if (dynamic_cast<const TAttLine *>(obj))
      editors |= kLineEditor;
   if (dynamic_cast<const TAttFill *>(obj))
      editors |= kFillEditor;
   if (dynamic_cast<const TAttPad *>(obj))
      editors |= kPadEditor;
      
   return editors;
}

}
