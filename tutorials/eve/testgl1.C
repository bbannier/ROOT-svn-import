// Demo / test of pad graphics in TGLViewer.

TEvePadFrame *epf1 = 0, *epf2 = 0, *epf3 = 0, *epf4 = 0;

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

   epf1 = new TEvePadFrame(gPad, "Pad 3D");
   epf1->RefMainTrans().Move3LF(-0.7, 0.5, 0);
   gEve->AddElement(epf1);

   epf2 = new TEvePadFrame(gPad, "Pad FBO");
   epf2->SetUseFBO(kTRUE);
   epf2->RefMainTrans().Move3LF(0.7, 0.5, 0);
   gEve->AddElement(epf2);

   epf3 = new TEvePadFrame(gPad, "Pad FBO Mipmap");
   epf3->SetUseFBO(kTRUE);
   epf3->SetUseMipmaps(kTRUE);
   epf3->RefMainTrans().Move3LF(-0.7, -0.5, 0);
   gEve->AddElement(epf3);

   epf4 = new TEvePadFrame(gPad, "Pad FBO LargeBuf");
   epf4->SetUseFBO(kTRUE);
   epf4->SetSizeFBO(2048);
   epf4->RefMainTrans().Move3LF(0.7, -0.5, 0);
   gEve->AddElement(epf4);

   gEve->Redraw3D(kTRUE);
}
