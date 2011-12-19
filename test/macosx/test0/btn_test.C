void print(const char * c, TGButton *b)
{
   std::cout<<c<<" w: "<<b->GetWidth()<<" h: "<<b->GetHeight()<<std::endl;
}

void btn_test()
{
   TGMainFrame *frame = new TGMainFrame(0, 400, 400);

   TGHorizontalFrame *nestedFrame = new TGHorizontalFrame(frame, 100, 100, kChildFrame | kFixedSize, 0xff0000);
   TGTextButton *btn = new TGTextButton(nestedFrame, "Try to press me");
//   print("a", btn);
   nestedFrame->AddFrame(btn, new TGLayoutHints(kLHintsCenterX | kLHintsCenterY, 2, 2, 2, 2));
//   print("b", btn);
   frame->AddFrame(nestedFrame, new TGLayoutHints(kLHintsCenterX | kLHintsCenterY, 2, 2, 2, 2));
//   print("c", btn);
   frame->MapSubwindows();
//   print("d", btn);
   frame->Layout();
//   print("e", btn);
   frame->MapRaised();
//   print("f", btn);
}
