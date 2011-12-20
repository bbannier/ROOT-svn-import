void btn_test()
{
   TGMainFrame *frame = new TGMainFrame(0, 400, 400);

   TGHorizontalFrame *nestedFrame = new TGHorizontalFrame(frame, 100, 100);
   TGTextButton *btn = new TGTextButton(nestedFrame, "Try to press me");
   nestedFrame->AddFrame(btn, new TGLayoutHints(kLHintsCenterX | kLHintsCenterY, 2, 2, 2, 2));
   frame->AddFrame(nestedFrame, new TGLayoutHints(kLHintsCenterX | kLHintsCenterY, 2, 2, 2, 2));
   frame->MapSubwindows();
   frame->Layout();
   frame->MapRaised();
}
