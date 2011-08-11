#include "generateFile.C"
#include "TStopwatch.h"
#include "TString.h"
#include "TSystem.h"
#include "TROOT.h"

// Definition of tests
//
//   #           Description
//
//   1           no compression, 20 files, 100000 evts/file, splitted
//   2           no compression, 200 files, 10000 evts/file, splitted
//   3           no compression, 20 files, 100000 evts/file, unsplitted
//   4           no compression, 200 files, 10000 evts/file, unsplitted
//   5           compression, 20 files, 100000 evts/file, splitted
//   6           compression, 200 files, 10000 evts/file, splitted
//   7           compression, 20 files, 100000 evts/file, unsplitted
//   8           compression, 200 files, 10000 evts/file, unsplitted

void WriteTest(const char *basedir = "", Int_t nf = 1, Long64_t ne = 1000,
               Int_t comp_level = 0, Int_t split_level = 99, FILE *fout = 0);

void WriteTests(const char *basedir, Int_t testset = 0)
{
   TString resdir(gROOT->GetVersion());
   resdir.ReplaceAll("/", "-"); resdir.ReplaceAll(".", "-");
   if (TString(gSystem->GetMakeExe()).Contains(gSystem->GetFlagsDebug())) resdir += "_dbg"; 
   resdir.Insert(0, "results/");
   if (gSystem->AccessPathName(resdir)) gSystem->mkdir(resdir, kTRUE);
   TString fnout = TString::Format("%s/writetest-%s.txt", resdir.Data(), gSystem->HostName());
   FILE *fout = fopen(fnout.Data(), "a");

   TString dir;
   if (testset == 1 || testset == 0) {
      // Write Test 1: no compression, 20 files, 100000 evts/file, splitted
      dir.Form("%s/data_20_c0_s99", basedir);
      WriteTest(dir, 20, 100000, 0, 99, fout);
   }
   if (testset == 2 || testset == 0) {
      // Write Test 2: no compression, 200 files, 10000 evts/file, splitted
      dir.Form("%s/data_200_c0_s99", basedir);
      WriteTest(dir, 200, 10000, 0, 99, fout);
   }
   if (testset == 3 || testset == 0) {
      // Write Test 3: no compression, 20 files, 100000 evts/file, unsplitted
      dir.Form("%s/data_20_c0_s0", basedir);
      WriteTest(dir, 20, 100000, 0, 0, fout);
   }
   if (testset == 4 || testset == 0) {
      // Write Test 4: no compression, 200 files, 10000 evts/file, unsplitted
      dir.Form("%s/data_200_c0_s0", basedir);
      WriteTest(dir, 200, 10000, 0, 0, fout);
   }
   if (testset == 5 || testset == 0) {
      // Write Test 5: compression, 20 files, 100000 evts/file, splitted
      dir.Form("%s/data_20_c1_s99", basedir);
      WriteTest(dir, 20, 100000, 1, 99, fout);
   }
   if (testset == 6 || testset == 0) {
      // Write Test 6: compression, 200 files, 10000 evts/file, splitted
      dir.Form("%s/data_200_c1_s99", basedir);
      WriteTest(dir, 200, 10000, 1, 99, fout);
   }
   if (testset == 7 || testset == 0) {
      // Write Test 7: compression, 20 files, 100000 evts/file, unsplitted
      dir.Form("%s/data_20_c1_s0", basedir);
      WriteTest(dir, 20, 100000, 1, 0, fout);
   }
   if (testset == 8 || testset == 0) {
      // Write Test 8: compression, 200 files, 10000 evts/file, unsplitted
      dir.Form("%s/data_200_c1_s0", basedir);
      WriteTest(dir, 200, 10000, 1, 0, fout);
   }

   if (fout) fclose(fout);
}


void WriteTest(const char *basedir, Int_t nf, Long64_t ne,
               Int_t comp_level, Int_t split_level, FILE *fout)
{

   fprintf(stderr, "Test: '%s'\r", gSystem->BaseName(basedir));

   if (gSystem->AccessPathName("filelists")) gSystem->MakeDirectory("filelists");
   TString fntxt = TString::Format("filelists/%s.txt", gSystem->BaseName(basedir));

   FILE *ftxt = fopen(fntxt.Data(), "w");
  
   if (gSystem->AccessPathName(basedir)) gSystem->MakeDirectory(basedir);

   TStopwatch swtc;
   TString fn;
   for (Int_t i = 1; i <= nf; i++) {
      fn.Form("%s/event-%s-%d.root", basedir,gSystem->HostName(), i);
      fprintf(stderr, "Writing '%s' ... %d to go   \r", fn.Data(), nf-i+1);
      generateFile(fn, ne, comp_level, split_level, 10, 5);
      if (ftxt) fprintf(ftxt, "%s\n", fn.Data());
   }
   swtc.Stop();

   if (ftxt) fclose(ftxt);

   // Get sizes
   Long64_t totsz = 0;
   FileStat_t st;
   for (Int_t i = 1; i <= nf; i++) {
      fn.Form("%s/event-%s-%d.root", basedir,gSystem->HostName(), i);
      if (gSystem->GetPathInfo(fn, st) == 0)
	 totsz += st.fSize;
      else
 	 Printf("WARNING: file '%s' cannor be stated", fn.Data());      
   }

   Double_t rate = (swtc.RealTime() > 0) ? totsz/1024/1024/swtc.RealTime() : -1.;
   Printf("WriteTest: '%s'   rt: %.3f s ct: %.3f s (%lld bytes, %.2f MB/s)",
          gSystem->BaseName(basedir), swtc.RealTime(), swtc.CpuTime(), totsz, rate);
   if (fout)
      fprintf(fout, "%s  %.3f  %.3f  %lld  %.2f\n",
              gSystem->BaseName(basedir), swtc.RealTime(), swtc.CpuTime(), totsz, rate);
}
