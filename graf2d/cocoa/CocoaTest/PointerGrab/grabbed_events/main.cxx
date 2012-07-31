#include <fstream>

#include "TApplication.h"
#include "TVirtualX.h"
#include "testframe.h"

int main(int argc, char ** argv)
{
   using namespace ROOT::CocoaTest;

   TApplication app("test_app", &argc, argv);
   
   TestFrame *mainFrame = new TestFrame(0, 500, 500, kMainFrame, 0xff0000);
   TestFrame *childFrame = new TestFrame(mainFrame, 300, 300, kChildFrame, 0xff6600);
   gVirtualX->MoveWindow(childFrame->GetId(), 100, 100);
   TestFrame *childChildFrame = new TestFrame(childFrame, 100, 100, kChildFrame, 0xff9900);
   gVirtualX->MoveWindow(childChildFrame->GetId(), 100, 100);
   gVirtualX->MapSubwindows(childFrame->GetId());
   gVirtualX->MapSubwindows(mainFrame->GetId());
   gVirtualX->MapRaised(mainFrame->GetId());

   //Case 1: window with id 5 is receiving enter/leave notify events.
   childFrame->AddInput(kEnterWindowMask | kLeaveWindowMask);
   childChildFrame->AddInput(kButtonPressMask);//this window initiate implicit grab!
   mainFrame->AddInput(kButtonReleaseMask | kPointerMotionMask);
   //Expected: before any button pressed, window 4 receives mouse motion events.
   //window 5 - enter/exit.

   app.Run();
}
