void testgl1()
{
   TEveManager::Create();
   gStyle->SetCanvasPreferGL(kTRUE);

   // --- Create an embedded canvas

   gEve->GetBrowser()->StartEmbedding(1);
   gROOT->ProcessLineFast("new TCanvas");
   gEve->GetBrowser()->StopEmbedding("XXX Canvas");

   TH1 *h = new TH1F("Fooesoto", "Barosana", 51, 0, 1);
   for (Int_t i=0; i<4000; ++i) {
      h->Fill(gRandom->Gaus(.25, .02), 0.04);
      h->Fill(gRandom->Gaus(.5, .1));
      h->Fill(gRandom->Gaus(.75, .02), 0.04);
   }
   h->Draw();

   TEveElement *e = new TEvePadFrame(gPad,"Foo");
   e->RefMainTrans();
   gEve->AddElement(e);
   gEve->Redraw3D(kTRUE);
}
