TEvePadFrame *epf1 = 0;
TEvePadFrame *epf2 = 0;

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

   epf1 = new TEvePadFrame(gPad, "Foo");
   epf1->RefMainTrans().MoveLF(1, -0.7);
   gEve->AddElement(epf1);

   epf2 = new TEvePadFrame(gPad, "Foo");
   epf2->SetUseFBO(kTRUE);
   epf2->RefMainTrans().MoveLF(1, 0.7);
   gEve->AddElement(epf2);

   gEve->Redraw3D(kTRUE);
}
