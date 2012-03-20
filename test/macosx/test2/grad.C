void grad()
{
   const Color_t colorIndices[] = {kRed, kOrange, 1003};
   const Double_t lengths[] = {1., 0.};
   new TColorGradient(colorIndices[2], TColorGradient::kGDVertical, 2, lengths, colorIndices, kTRUE);

   TCanvas *cnv = new TCanvas("cnv", "gradient_test", 100, 100, 600, 600);
   cnv->cd(1);

   TH1F * hist = new TH1F("a", "b", 20, -3., 3.);
   hist->SetFillColor(colorIndices[2]);
   hist->FillRandom("gaus", 100000);
   hist->Draw();
}
