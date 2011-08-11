#include "TChain.h"
#include "TFile.h"
#include "TFileCollection.h"
#include "THashList.h"
#include "TProof.h"
#include "TQueryResult.h"
#include "TStopwatch.h"
#include "TString.h"
#include "TSystem.h"
#include "TROOT.h"

void ParseTag(const char *tag, Int_t &nf, Int_t &comp, Int_t &split, TString &fnlist);

void ReadTest(const char *basedir, Bool_t cold = kTRUE, const char *proof = 0, Bool_t rebuild = kFALSE, Int_t pchszmb = -1)
{

   // Get test tag
   TString testtag = gSystem->BaseName(basedir);
   
   // Parse test tag to get relevant config parameters
   TString fnlist;
   Int_t nf, comp, split;
   ParseTag(testtag, nf, comp, split, fnlist);

   // Read selector
   TString sel = (split) ? "ReadSelSplit" : "ReadSel";
   const char *aclicc = (rebuild) ? "++" : "+";
   TString selload = TString::Format("src/%s.cxx%s", sel.Data(), aclicc);
   
   TString resdir(gROOT->GetVersion());
   resdir.ReplaceAll("/", "-"); resdir.ReplaceAll(".", "-");
   if (TString(gSystem->GetMakeExe()).Contains(gSystem->GetFlagsDebug())) resdir += "_dbg"; 
   resdir.Insert(0, "results/");
   if (gSystem->AccessPathName(resdir)) gSystem->mkdir(resdir, kTRUE);
   TString fnout = TString::Format("%s/readtest-%s_%s.txt", resdir.Data(), gSystem->HostName(), testtag.Data());
   FILE *fout = fopen(fnout.Data(), "a");

   TStopwatch swtc;
   swtc.Stop();
   Long64_t ref_rb, totsz = -1;
   if (!proof || (proof && strlen(proof) <= 0)) {
      // Local run
      TFileCollection *fc = new TFileCollection("","",fnlist.Data());
      TChain *c = new TChain("rt");
      c->AddFileInfoList(fc->GetList());

      gROOT->ProcessLine(TString::Format(".L %s", selload.Data()));
      
      ref_rb = TFile::GetFileBytesRead();
      
      swtc.Start();
      c->Process(sel);
      swtc.Stop();

      totsz = TFile::GetFileBytesRead() - ref_rb;
      ref_rb = TFile::GetFileBytesRead();
   } else {
      // Proof run
      TProof *p = gProof;
      if (!p) {
         p = TProof::Open(proof);
      } else {
         TString sp(proof);
         sp.ReplaceAll("workers=", "");
         Printf("sp: '%s' (%s)", sp.Data(), proof);
         if (sp.IsDigit()) p->SetParallel(sp.Atoi());
      }
      if (p && p->IsValid()) {
         if (pchszmb > 0) p->SetParameter("PROOF_CacheSize", (Long64_t)(1024*1024*pchszmb));
         p->UploadPackage("packages/RTEvent.par");
         p->EnablePackage("RTEvent", kTRUE);
         p->Load(selload.Data());
         swtc.Start();
         p->Process(testtag.Data(), sel);
         swtc.Stop();
         if (p->GetQueryResult())
            totsz = p->GetQueryResult()->GetBytes();
         p->Close();
         delete p;
      }
   }
   
   Double_t rate = (swtc.RealTime() > 0) ? totsz/1024/1024/swtc.RealTime() : -1.;
   Printf("ReadTest: '%s'   rt: %.3f s ct: %.3f s (%lld bytes, %.2f MB/s) %d %s",
          gSystem->BaseName(basedir), swtc.RealTime(), swtc.CpuTime(), totsz, rate, cold, proof ? proof : "");
   if (fout)
      fprintf(fout, "%s  %.3f  %.3f  %lld  %.2f %d %s\n",
                    testtag.Data(), swtc.RealTime(), swtc.CpuTime(), totsz, rate, cold, proof ? proof : "");

   if (fout) fclose(fout);
}

void ReadTests(const char *basedir, Bool_t dowarm = kFALSE, Bool_t rebuild = kFALSE, Int_t pchszmb = -1)
{

   // Basic ROOT cold read
   gROOT->ProcessLine(TString::Format("releaseCache(\"%s\")", basedir));
   ReadTest(basedir, kFALSE, 0, rebuild);
   // Basic ROOT warm read
   if (dowarm) ReadTest(basedir, kTRUE);
   // Basic PROOF cold reads
   TProof *p = TProof::Open("lite://");
   if (!p || (p && !p->IsValid())) return;
   Int_t nw = p->GetParallel();
   Int_t i = 0;
   for (i = 2; i <= nw; i++) { 
      gROOT->ProcessLine(TString::Format("releaseCache(\"%s\")", basedir));
      ReadTest(basedir, kFALSE, TString::Format("workers=%d",i), rebuild, pchszmb);
      rebuild = kFALSE;
   }
   // Basic PROOF warm reads
   if (dowarm) {
      for (i = 2; i <= nw; i++) { 
         ReadTest(basedir, kTRUE, TString::Format("workers=%d",i), pchszmb);
      }
   }   
}


void ParseTag(const char *tag, Int_t &nf, Int_t &comp, Int_t &split, TString &fnlist)
{
   // Init outputs
   nf = -1;
   comp = -1;
   split = -1;
   fnlist = "";
   
   TString tg(tag), t;
   // Get rid of 'data_'
   tg.ReplaceAll("data_", "");
   // Tokenize over '_'
   Ssiz_t from = 0;
   while (tg.Tokenize(t, from, "_")) {
      if (t.BeginsWith("c")) {
         t.Remove(0,1);
         if (t.IsDigit()) comp = t.Atoi();
      } else if (t.BeginsWith("s")) {
         t.Remove(0,1);
         if (t.IsDigit()) split = t.Atoi();
      } else {
         if (t.IsDigit()) nf = t.Atoi();
      }
   }
   fnlist.Form("filelists/%s.txt", tag);
   // Done
   Printf("ParseTag: '%s' -> nf: %d, comp: %d, split: %d, fnlist: '%s'", tag, nf, comp, split, fnlist.Data());
}


