#include <fstream>

#include "TApplication.h"
#include "TVirtualX.h"
#include "testframe.h"

int main(int argc, char ** argv)
{
   //All test cases are described in

   using namespace ROOT::CocoaTest;

   TApplication app("test_app", &argc, argv);
   
   TestFrame *mainFrame = new TestFrame(0, 500, 500, kMainFrame, 0xff0000);
   TestFrame *childFrame = new TestFrame(mainFrame, 400, 300, kChildFrame, 0xff6600);
   gVirtualX->MoveWindow(childFrame->GetId(), 100, 100);
   TestFrame *childChildFrame = new TestFrame(childFrame, 300, 100, kChildFrame, 0xff9900);
   gVirtualX->MoveWindow(childChildFrame->GetId(), 100, 100);
   gVirtualX->MapSubwindows(childFrame->GetId());
   gVirtualX->MapSubwindows(mainFrame->GetId());
   gVirtualX->MapRaised(mainFrame->GetId());

   //Test case 1: "middle" window has a grab, and
#ifdef TEST1
   mainFrame->AddInput(kEnterWindowMask | kLeaveWindowMask);
   childFrame->AddInput(kEnterWindowMask | kLeaveWindowMask);
   childChildFrame->AddInput(kEnterWindowMask | kLeaveWindowMask);
#elif defined(TEST2)
   //Masks for the first window:
   mainFrame->AddInput(kEnterWindowMask | kLeaveWindowMask);
   childFrame->AddInput(kEnterWindowMask | kLeaveWindowMask);
   childChildFrame->AddInput(kEnterWindowMask | kLeaveWindowMask);


   //Create the second window:
   TestFrame *main2 = new TestFrame(0, 500, 500, kMainFrame, 0xffff00);
   TestFrame *child2 = new TestFrame(main2, 300, 300, kChildFrame, 0xff00);
   
   main2->AddInput(kEnterWindowMask | kLeaveWindowMask);
   child2->AddInput(kEnterWindowMask | kLeaveWindowMask);
   //Place top-level windows side-by-side.
   gVirtualX->MoveWindow(mainFrame->GetId(), 600, 600);
   gVirtualX->MoveWindow(main2->GetId(), 1100, 600);
   
   gVirtualX->MapSubwindows(main2->GetId());
   gVirtualX->MapRaised(main2->GetId());
#elif defined (TEST3)
   mainFrame->AddInput(kEnterWindowMask | kLeaveWindowMask);
   //childFrame->AddInput(kEnterWindowMask | kLeaveWindowMask | kButtonPressMask);
   childFrame->AddInput(kEnterWindowMask | kLeaveWindowMask | kButtonPressMask | kButtonReleaseMask);
   childChildFrame->AddInput(kEnterWindowMask | kLeaveWindowMask);
#elif defined (TEST4)
   mainFrame->AddInput(kEnterWindowMask | kLeaveWindowMask);
   childFrame->AddInput(kEnterWindowMask | kLeaveWindowMask | kButtonPressMask | kButtonReleaseMask);//this window can activate implicit grab.
   childChildFrame->AddInput(kEnterWindowMask | kLeaveWindowMask);


   //Create the second window:
   TestFrame *main2 = new TestFrame(0, 500, 500, kMainFrame, 0xffff00);
   TestFrame *child2 = new TestFrame(main2, 300, 300, kChildFrame, 0xff00);
   
   main2->AddInput(kEnterWindowMask | kLeaveWindowMask | kButtonPressMask | kButtonReleaseMask);//this window can activate implicit grab.
   child2->AddInput(kEnterWindowMask | kLeaveWindowMask);
   //Place top-level windows side-by-side.
   gVirtualX->MoveWindow(mainFrame->GetId(), 600, 600);
   gVirtualX->MoveWindow(main2->GetId(), 1100, 600);
   
   gVirtualX->MapSubwindows(main2->GetId());
   gVirtualX->MapRaised(main2->GetId());
#elif defined (TEST5)
   mainFrame->AddInput(kEnterWindowMask | kLeaveWindowMask);
   childFrame->AddInput(kEnterWindowMask | kLeaveWindowMask);
   childChildFrame->AddInput(kEnterWindowMask | kLeaveWindowMask);

   gVirtualX->GrabButton(childFrame->GetId(), kButton1, kAnyModifier, kButtonPressMask | kButtonReleaseMask | kEnterWindowMask | kLeaveWindowMask, kNone, kNone, kTRUE);
#elif defined (TEST6)
   mainFrame->AddInput(kEnterWindowMask | kLeaveWindowMask);
   childFrame->AddInput(kEnterWindowMask | kLeaveWindowMask);
   childChildFrame->AddInput(kEnterWindowMask | kLeaveWindowMask);

   gVirtualX->GrabButton(childFrame->GetId(), kButton1, kAnyModifier, kNone, kNone, kNone, kTRUE);
#elif defined (TEST7)
   mainFrame->AddInput(kEnterWindowMask | kLeaveWindowMask);
   childChildFrame->AddInput(kEnterWindowMask | kLeaveWindowMask);

   gVirtualX->GrabButton(childFrame->GetId(), kButton1, kAnyModifier, kEnterWindowMask | kLeaveWindowMask, kNone, kNone, kTRUE);
#elif defined (TEST8)
   mainFrame->AddInput(kEnterWindowMask | kLeaveWindowMask);
   childFrame->AddInput(kEnterWindowMask | kLeaveWindowMask);
   childChildFrame->AddInput(kEnterWindowMask | kLeaveWindowMask);

   //Create the second window:
   TestFrame *main2 = new TestFrame(0, 500, 500, kMainFrame, 0xffff00);
   TestFrame *child2 = new TestFrame(main2, 300, 300, kChildFrame, 0xff00);
   
   main2->AddInput(kEnterWindowMask | kLeaveWindowMask);//this window can activate implicit grab.
   child2->AddInput(kEnterWindowMask | kLeaveWindowMask);
   //Place top-level windows side-by-side.
   gVirtualX->MoveWindow(mainFrame->GetId(), 600, 600);
   gVirtualX->MoveWindow(main2->GetId(), 1100, 600);
   
   gVirtualX->MapSubwindows(main2->GetId());
   gVirtualX->MapRaised(main2->GetId());   

   gVirtualX->GrabButton(childFrame->GetId(), kButton1, kAnyModifier, kNone, kNone, kNone, kTRUE);
#elif defined (TEST9)
   mainFrame->AddInput(kEnterWindowMask | kLeaveWindowMask);
   childFrame->AddInput(kEnterWindowMask | kLeaveWindowMask | kButtonPressMask); //This will init at first implicit grab, which later will be changed by active grab.
   childChildFrame->AddInput(kEnterWindowMask | kLeaveWindowMask);
#elif defined (TEST10)
   mainFrame->AddInput(kButtonPressMask | kButtonReleaseMask);//this can be commented or one of options removed.
//   mainFrame->AddInput(kButtonReleaseMask);//additional option to test.
//   mainFrame->AddInput(kButtonPressMask);
   //childFrame->AddInput(kButtonPressMask | kButtonReleaseMask);//additional option to test.
   childChildFrame->AddInput(kButtonPressMask | kButtonReleaseMask);
   gVirtualX->GrabButton(childFrame->GetId(), kButton1, kAnyModifier, kButtonPressMask | kButtonReleaseMask, kNone, kNone, kTRUE);
#elif defined (TEST11)
   mainFrame->AddInput(kEnterWindowMask | kLeaveWindowMask);
   childFrame->AddInput(kEnterWindowMask | kLeaveWindowMask | kButtonPressMask); //This will init at first implicit grab, which later will be changed by active grab.
   childChildFrame->AddInput(kEnterWindowMask | kLeaveWindowMask);
#elif defined (TEST12)
   mainFrame->AddInput(kButtonPressMask);
//   childFrame->AddInput(kButtonReleaseMask);
   childFrame->AddInput(kButtonReleaseMask | kButtonPressMask);
   childChildFrame->AddInput(kButtonPressMask | kButtonReleaseMask);
#elif defined (TEST13)
   mainFrame->AddInput(kPointerMotionMask);
   childChildFrame->AddInput(kButtonMotionMask | kButtonPressMask);
#elif defined (TEST14)
//   mainFrame->AddInput(kPointerMotionMask);
   childChildFrame->AddInput(kPointerMotionMask);
   gVirtualX->GrabButton(childFrame->GetId(), kButton1, kAnyModifier, kButtonMotionMask, kNone, kNone, kTRUE);
#elif defined (TEST15)
   gVirtualX->GrabButton(childFrame->GetId(), kButton1, kAnyModifier, kNone, kNone, kNone, kTRUE);
   childChildFrame->AddInput(kPointerMotionMask);
#elif defined (TEST16)
   childFrame->AddInput(kButtonPressMask);
   childChildFrame->AddInput(kPointerMotionMask);
#elif defined (TEST17)
   mainFrame->AddInput(kPointerMotionMask);
   childFrame->AddInput(kButtonPressMask);
   childChildFrame->AddInput(kPointerMotionMask);

   //Create the second window:
   TestFrame *main2 = new TestFrame(0, 500, 500, kMainFrame, 0xffff00);
   TestFrame *child2 = new TestFrame(main2, 300, 300, kChildFrame, 0xff00);
   
   main2->AddInput(kPointerMotionMask);//this window can activate implicit grab.
   child2->AddInput(kButtonMotionMask);
   //Place top-level windows side-by-side.
   gVirtualX->MoveWindow(mainFrame->GetId(), 600, 600);
   gVirtualX->MoveWindow(main2->GetId(), 1100, 600);
   
   gVirtualX->MapSubwindows(main2->GetId());
   gVirtualX->MapRaised(main2->GetId());
#endif

   app.Run();
}
