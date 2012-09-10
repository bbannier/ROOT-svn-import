#include "TApplication.h"
#include "TVirtualX.h"

#include "testframe.h"

#define TEST1

int main(int argc, char ** argv)
{
   //All test cases are described in

   using namespace ROOT::CocoaTest;

   TApplication app("test_app", &argc, argv);

#ifdef TEST1
   TestFrame *mainFrame = new TestFrame(0, 1500, 1500, kMainFrame, 0xff0000);
   
   TestFrame *childFrame1 = new TestFrame(mainFrame, 1400, 1300, kChildFrame, 0xff6600);
   gVirtualX->MoveWindow(childFrame1->GetId(), 100, 100);
   childFrame1->AddInput(kButtonPressMask);
   
   TestFrame *childFrame2 = new TestFrame(mainFrame, 1250, 1250, kChildFrame, 0xff0066);
   gVirtualX->MoveWindow(childFrame2->GetId(), 0, 250);
   
   gVirtualX->MapSubwindows(mainFrame->GetId());
   gVirtualX->MapRaised(mainFrame->GetId());
#elif defined(TEST2)

#endif

   app.Run();
}
