#include "TVirtualPadPainter.h"
#include "TPluginManager.h"

ClassImp(TVirtualPadPainter)

//______________________________________________________________________________
TVirtualPadPainter::~TVirtualPadPainter()
{
   //Virtual dtor.
}

//______________________________________________________________________________
TVirtualPadPainter *TVirtualPadPainter::PadPainter(TVirtualPad *pad, Option_t *type)
{
   // Create a Viewer 3D of specified type.

   TVirtualPadPainter *painter = 0;
   TPluginHandler *h = gPluginMgr->FindHandler("TVirtualPadPainter", type);
   
   if (h && h->LoadPlugin() != -1)
      painter = (TVirtualPadPainter *) h->ExecPlugin(1, pad);

   return painter;
}
