{
   // Remove old link to package 'RTEvent'
   if (!gSystem->AccessPathName("RTEvent")) gSystem->Unlink("RTEvent");
   // Add local dirs to macro path
   TString mp(TROOT::GetMacroPath());
   Ssiz_t icol = mp.Index(".:");
   if (icol == kNPOS) icol = -2;
   mp.Insert(icol+2, TString::Format("%s/macros:%s/src:", gSystem->Getenv("KSDIR"), gSystem->Getenv("KSDIR")));
   TROOT::SetMacroPath(mp);
   // Load most common macros
   gROOT->ProcessLine(".L loadPackage.C+");
   gROOT->ProcessLine("loadPackage(\"RTEvent\")");
   gROOT->ProcessLine(".L releaseCache.C+");
   gROOT->ProcessLine(".L WriteTest.C+");
   gROOT->ProcessLine(".L ReadTest.C+");
}
