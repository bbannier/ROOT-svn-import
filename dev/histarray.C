// This test checks the I/O backward compatibility of classes TArray(C|S|I|L|L64|F|D)
//    after the introduction of the template class TArrayTi
#include <cassert>
#include <typeinfo>
#include "Riostream.h"
#include "TH1.h"
#include "TH1F.h"
#include "TH1C.h"
#include "TH2D.h"
#include "TH2I.h"
#include "TH3S.h"
#include "TFile.h"

void write(const char *filename = "histarray.root")
{
   TFile *f = TFile::Open(filename, "RECREATE");
   
   TH1 *hc = new TH1C("hc", "hc", 100, 0, 20);
   hc->SetBinContent(100, 45);
   hc->SetBinContent(1, -20);
   hc->AddBinContent(2); hc->AddBinContent(50, 14);
   hc->Write();

   TH1* hf = new TH1F("hf", "hf", 10, 0, 10);
   hf->Write();

   TH2* hd = new TH2D("hd", "hd", 30, -2, 5, 10, 0, 1);
   hd->Write();

   TH3* hs = new TH3S("hs", "hs", 100, 0, 4, 35, -2, 4, 15, -6, 3);
   hs->Write();

   TH2* hi = new TH2I("hi", "hi", 46, 0, 6, 14, -10, -9);
   hi->Write();

   f->Close();
}


void read(const char *filename = "histarray.root")
{
   TFile *f = TFile::Open(filename, "READ");

   TH1 *hc; f->GetObject("hc", hc);
   assert(hc != NULL);
   assert(hc->GetBinContent(100) == 45);
   assert(hc->GetBinContent(1) == -20);
   assert(hc->GetBinContent(2) == 15);
   for(int i = 3; i < 100; ++i) assert(hc->GetBinContent(i) == 0);


   TH1F *hf; f->GetObject("hf", hf);
   assert(hf != NULL);
   TH1 *hd; f->GetObject("hd", hd);
   assert(hd != NULL);
   TH1 *hs; f->GetObject("hs", hs);
   assert(hs != NULL);
   TH1 *hi; f->GetObject("hi", hi);
   assert(hi != NULL);

   f->Close();
}


int runhistarray(const char* filename = "histarray.root")
{
   read();
   return 0;
}




