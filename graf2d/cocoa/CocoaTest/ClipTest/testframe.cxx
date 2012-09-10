#include <iostream>
#include <cassert>

#include "TVirtualX.h"
#include "RConfigure.h"

#include "testframe.h"

namespace ROOT {
namespace CocoaTest {

Window_t TestFrame::currentWindowID_ = 4;
FontStruct_t TestFrame::font_ = kNone;
GContext_t TestFrame::textContext_ = kNone;

//_____________________________________________________
TestFrame::TestFrame(TestFrame *parent, UInt_t width, UInt_t heihght,
                     UInt_t options, Pixel_t background)
               : TGFrame(parent, width, heihght, options, background)
{
#ifdef R__HAS_COCOA
   windowID_ = GetId();
#else
   windowID_ = currentWindowID_++;
#endif

   if (font_ == kNone) {//Init font.
      assert(textContext_ == kNone);
#ifdef R__HAS_COCOA
      font_ = gVirtualX->LoadQueryFont("-*-courier-*-*-*-*-14");
#else
      font_ = gVirtualX->LoadQueryFont("fixed");
#endif
      GCValues_t gcVals;
      gcVals.fFont = gVirtualX->GetFontHandle(font_);

      gcVals.fMask = kGCFont | kGCForeground;
      textContext_ = gVirtualX->CreateGC(GetId(), &gcVals);
   }
}

//_____________________________________________________
TestFrame::~TestFrame()
{
}

//_____________________________________________________
void TestFrame::DoRedraw()
{
   TGFrame::DoRedraw();
   
   const TString text(TString::Format("id : %u, w : %u, h : %u", unsigned(windowID_), unsigned(GetWidth()), unsigned(GetHeight())));
   gVirtualX->DrawString(GetId(), textContext_, 0, 30, text.Data(), text.Length());
}

//_____________________________________________________
Bool_t TestFrame::HandleButton(Event_t *buttonEvent)
{
   assert(buttonEvent);
   
   if (buttonEvent->fType == kButtonPress) {
      //Do a trick here: redraw this window
      //without repainting any other window
      //(test for a clip - other windows should not
      //be affected).
      std::cout<<"Requesting redraw!\n";

      static const ULong_t pixels[] = {0xff0000, 0xff00, 0xff, 0xffff00, 0xff00ff, 0xffff};
      static const unsigned nPixels = sizeof pixels / sizeof pixels[0];
      static unsigned index = 0;
      
      if (index + 1 == nPixels)
         index = 0;
      else
         ++index;
         
      gVirtualX->SetWindowBackground(GetId(), pixels[index]);
      gClient->NeedRedraw(this);
   }

   return kTRUE;
}

}
}
