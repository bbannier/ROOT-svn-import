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
#ifdef TEST1
   childFrame->AddInput(kEnterWindowMask | kLeaveWindowMask);
   childChildFrame->AddInput(kButtonPressMask);//this window initiate implicit grab!
   mainFrame->AddInput(kButtonReleaseMask | kPointerMotionMask);
   //Expected: before any button pressed, window 4 receives mouse motion events.
   //window 5 - enter/exit.
   
#elif defined(TEST2)
   //Case 2: crossing events with grab active and owner == false (implicit grab).
   childFrame->AddInput(kEnterWindowMask | kLeaveWindowMask);
   childChildFrame->AddInput(kEnterWindowMask | kLeaveWindowMask | kButtonPressMask);//button press mask will initiate implicit grab on this view.
   mainFrame->AddInput(kEnterWindowMask | kLeaveWindowMask);
   //Expected: while no button is pressed, every window report enter/leave notify, as usually.
   //If button is pressed in any window except window with id 6, no enter/leave notify events are reported
   //(pseudo grab with 'root'). If button is pressed in window with id == 6, exit/leave notify are
   //reported for window 6 only. After button was released, leave/enter notify are generated on ungrab (if needed)
   //and so on.
#elif defined(TEST3)
   //Case 3: pointer grab on window and enter/leave notify events.
   mainFrame->AddInput(kEnterWindowMask | kLeaveWindowMask);
   childFrame->AddInput(kEnterWindowMask | kLeaveWindowMask);
   
   //On a button press we will setup pointer grab with owner_events == false (see testframe.cxx).
   //On a button release pointer grab is cancelled.
   childChildFrame->AddInput(kEnterWindowMask | kLeaveWindowMask | kButtonPressMask | kButtonReleaseMask);
#endif

   app.Run();
}
