#ifndef TESTFRAME_INCLUDED
#define TESTFRAME_INCLUDED

#include "GuiTypes.h"
#include "TGFrame.h"

namespace ROOT {
namespace CocoaTest {

class TestFrame : public TGFrame {
public:
   TestFrame(TestFrame *parent, UInt_t width, UInt_t height, UInt_t options, Pixel_t background);
   ~TestFrame();

   void DoRedraw();

   //Event handlers:
   Bool_t HandleButton(Event_t *buttonEvent);

private:

   TestFrame(const TestFrame &rhs);
   TestFrame &operator = (const TestFrame &rhs);
   
   //To be used by all test frames.
   Window_t windowID_;

   static Window_t currentWindowID_;
   static FontStruct_t font_;
   static GContext_t textContext_;
};

}
}

#endif
