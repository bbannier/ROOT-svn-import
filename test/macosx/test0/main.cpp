#include "TApplication.h"
#include "TGButton.h"
#include "TGFrame.h"

int main(int argc, char ** argv)
{
   TApplication app("test_app", &argc, argv);
   TGMainFrame *frame = new TGMainFrame(0, 400, 400);

   TGHorizontalFrame *nestedFrame = new TGHorizontalFrame(frame, 400, 400);
   TGTextButton *btn = new TGTextButton(nestedFrame, "Try to press me");
   nestedFrame->AddFrame(btn, new TGLayoutHints(kLHintsCenterX | kLHintsCenterY, 2, 2, 2, 2));
   frame->AddFrame(nestedFrame, new TGLayoutHints(kLHintsCenterX | kLHintsCenterY, 2, 2, 2, 2));
   frame->MapSubwindows();
   frame->Layout();
   frame->MapRaised();

   app.Run();
}
