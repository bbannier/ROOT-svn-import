void red_square()
{
   //Thanks to Bertrand, who helped me to fix layouts and geometry in this small macro.

   //Simple macro to test background color,
   //TGFrame::DoRedraw (and TGCocoa::ClearArea), 
   //ConfigureNotify + Expose events (implicit),
   //RootQuartzView's behavior,
   //TGCocoa::MoveResize and nested frames geometry.

   TGMainFrame *frame = new TGMainFrame(0, 400, 400);

   TGHorizontalFrame *nestedFrame = new TGHorizontalFrame(frame, 200, 200, kChildFrame | kFixedSize, 0xff0000);
   frame->AddFrame(nestedFrame, new TGLayoutHints(kLHintsCenterX | kLHintsCenterY, 2, 2, 2, 2));
   frame->MapSubwindows();
   frame->Layout();
   frame->MapRaised();
}
