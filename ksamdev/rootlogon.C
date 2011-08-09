{
  // Add local dirs to macro path
   TString mp(TROOT::GetMacroPath());
   Ssiz_t icol = mp.Index(".:");
   if (icol == kNPOS) icol = -2;
   mp.Insert(icol+2, TString::Format("%s/macros:%s/src:", gSystem->Getenv("KSDIR"), gSystem->Getenv("KSDIR")));
   TROOT::SetMacroPath(mp);
}
