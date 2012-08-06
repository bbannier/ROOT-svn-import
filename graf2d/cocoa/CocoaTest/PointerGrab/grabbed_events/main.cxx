#include <fstream>

#include "TApplication.h"
#include "TVirtualX.h"
#include "testframe.h"

#define TEST1 1

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

   //Test case 1: "middle" window has a grab, and
#ifdef TEST1
   //"implicit grab on a 'root'":
   //without any button pressed, you receive leave/enter notify events as
   //pointer moves from one window to another (as expected).
   //With button press, before the same button is released - no crossing events are generated.
   mainFrame->AddInput(kEnterWindowMask | kLeaveWindowMask);
   childFrame->AddInput(kEnterWindowMask | kLeaveWindowMask);
   childChildFrame->AddInput(kEnterWindowMask | kLeaveWindowMask);
#elif defined(TEST2)
#endif

   app.Run();
}
