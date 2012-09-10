#include "TApplication.h"
#include "TVirtualX.h"

#include "testframe.h"

#define TEST6

int main(int argc, char ** argv)
{
   //All test cases are described in

   using namespace ROOT::CocoaTest;

   TApplication app("test_app", &argc, argv);

#ifdef TEST1
   TestFrame *mainFrame = new TestFrame(0, 500, 500, kMainFrame, 0xff0000);
   mainFrame->AddInput(kButtonPressMask);
   
   TestFrame *childFrame1 = new TestFrame(mainFrame, 400, 300, kChildFrame, 0xff6600);
   gVirtualX->MoveWindow(childFrame1->GetId(), 50, 100);
   mainFrame->AddInput(kButtonPressMask);
   
   gVirtualX->MapSubwindows(mainFrame->GetId());
   gVirtualX->MapRaised(mainFrame->GetId());
#elif defined(TEST2)
   TestFrame *mainFrame = new TestFrame(0, 500, 500, kMainFrame, 0xff0000);
   mainFrame->AddInput(kButtonPressMask);
   
   TestFrame *childFrame1 = new TestFrame(mainFrame, 500, 500, kChildFrame, 0xff6600);
   gVirtualX->MoveWindow(childFrame1->GetId(), 0, 0);

   TestFrame *childFrame2 = new TestFrame(mainFrame, 100, 100, kChildFrame, 0xff66ee);
   gVirtualX->MoveWindow(childFrame2->GetId(), 50, 50);

   gVirtualX->MapSubwindows(mainFrame->GetId());
   gVirtualX->MapRaised(mainFrame->GetId());
#elif defined(TEST3)
   TestFrame *mainFrame = new TestFrame(0, 500, 500, kMainFrame, 0xff0000);
   mainFrame->AddInput(kButtonPressMask);
   
   TestFrame *childFrame1 = new TestFrame(mainFrame, 500, 500, kChildFrame, 0xff6600);
   gVirtualX->MoveWindow(childFrame1->GetId(), -50, 0);

   TestFrame *childFrame2 = new TestFrame(mainFrame, 100, 100, kChildFrame, 0xff66ee);
   gVirtualX->MoveWindow(childFrame2->GetId(), 50, 50);

   gVirtualX->MapSubwindows(mainFrame->GetId());
   gVirtualX->MapRaised(mainFrame->GetId());
#elif defined(TEST4)
   TestFrame *mainFrame = new TestFrame(0, 500, 500, kMainFrame, 0xff0000);
   mainFrame->AddInput(kButtonPressMask);
   
   TestFrame *childFrame1 = new TestFrame(mainFrame, 500, 500, kChildFrame, 0xff6600);
   gVirtualX->MoveWindow(childFrame1->GetId(), -50, 0);

   TestFrame *childFrame2 = new TestFrame(mainFrame, 100, 100, kChildFrame, 0xff66ee);
   gVirtualX->MoveWindow(childFrame2->GetId(), 400, 50);

   gVirtualX->MapSubwindows(mainFrame->GetId());
   gVirtualX->MapRaised(mainFrame->GetId());

#elif defined(TEST5)

   TestFrame *mainFrame = new TestFrame(0, 500, 500, kMainFrame, 0xff0000);
   mainFrame->AddInput(kButtonPressMask);
   
   TestFrame *childFrame1 = new TestFrame(mainFrame, 500, 200, kChildFrame, 0xff6600);
   gVirtualX->MoveWindow(childFrame1->GetId(), 0, 150);

   TestFrame *childFrame2 = new TestFrame(mainFrame, 200, 500, kChildFrame, 0x6500ff);
   gVirtualX->MoveWindow(childFrame2->GetId(), 150, 00);

   gVirtualX->MapSubwindows(mainFrame->GetId());
   gVirtualX->MapRaised(mainFrame->GetId());
   
#elif defined(TEST6)
   TestFrame *mainFrame = new TestFrame(0, 500, 500, kMainFrame, 0xff0000);
   mainFrame->AddInput(kButtonPressMask);
   
   TestFrame *childFrame1 = new TestFrame(mainFrame, 100, 100, kChildFrame, 0xff6600);
   gVirtualX->MoveWindow(childFrame1->GetId(), 100, 100);

   TestFrame *childFrame2 = new TestFrame(mainFrame, 100, 100, kChildFrame, 0x6500ff);
   gVirtualX->MoveWindow(childFrame2->GetId(), 100, 300);

   TestFrame *childFrame3 = new TestFrame(mainFrame, 100, 100, kChildFrame, 0x6500ff);
   gVirtualX->MoveWindow(childFrame3->GetId(), 300, 100);

   TestFrame *childFrame4 = new TestFrame(mainFrame, 100, 100, kChildFrame, 0x6500ff);
   gVirtualX->MoveWindow(childFrame4->GetId(), 300, 300);


   gVirtualX->MapSubwindows(mainFrame->GetId());
   gVirtualX->MapRaised(mainFrame->GetId());


#endif

   app.Run();
}
