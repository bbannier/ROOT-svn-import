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
   
   gVirtualX->DrawString(windowID_, textContext_, 0, 30, text.Data(), text.Length());
}

//_____________________________________________________
Bool_t TestFrame::HandleCrossing(Event_t *crossingEvent)
{
   assert(crossingEvent);

   std::cout<<"----------BEGIN------------\n";
   PrintFrameInfo();
   
   if (crossingEvent->fType == kEnterNotify)
      std::cout<<"Enter notify event:\n";
   else
      std::cout<<"Leave notify event:\n";

   PrintEventCoordinates(crossingEvent);
   std::cout<<"-----------END-----------\n";
   
   return kTRUE;
}

//_____________________________________________________
void TestFrame::PrintFrameInfo()const
{
   std::cout<<"this == "<<this<<" window id == "<<windowID_<<std::endl;
}

//_____________________________________________________
void TestFrame::PrintEventCoordinates(const Event_t *event)const
{
   assert(event);

   std::cout<<"event.x == "<<event->fX<<" event.y == "<<event->fY<<std::endl;
   std::cout<<"event.xroot == "<<event->fXRoot<<" event.yroot == "<<event->fYRoot<<std::endl;
}

}
}
