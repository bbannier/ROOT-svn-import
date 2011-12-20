void slider_test()
{
   TGMainFrame *frame = new TGMainFrame(0, 400, 400);
   TGHorizontalFrame *nestedFrame = new TGHorizontalFrame(frame, 150, 150);
   TGHSlider *slider = new TGHSlider(nestedFrame, 150, kSlider2 | kScaleBoth);
   slider->SetRange(0, 3);
   slider->SetPosition(2);
   nestedFrame->AddFrame(slider, new TGLayoutHints(kLHintsCenterX | kLHintsCenterY, 2, 2, 2, 2));
   frame->AddFrame(nestedFrame, new TGLayoutHints(kLHintsCenterX | kLHintsCenterY, 2, 2, 2, 2));
   frame->MapSubwindows();
   frame->Layout();
   frame->MapRaised();
}
