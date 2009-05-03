void P010_TGLPadPainter()
{
   gPluginMgr->AddHandler("TGLPadPainter", "*", "TGLPadPainter",
      "RGL", "TGLPadPainter(TCanvas *)");
}
