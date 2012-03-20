void grad()
{
   const Color_t colorIndices[5] = {kRed, kOrange, kYellow, kGreen, 1002};
   const Double_t lengths[4] = {1., 0.8, 0.5, 0.};
   new TColorGradient(colorIndices[4], TColorGradient::kGDVertical, 4, lengths, colorIndices, kTRUE);

   TCanvas *cnv = new TCanvas("cnv", "gradient_test", 100, 100, 600, 600);

   TH1F * hist2 = new TH1F("a", "b", 20, -3., 3.);
   hist2->SetFillColor(colorIndices[4]);
   hist2->FillRandom("gaus", 100000);
   hist2->Draw();
}
