#include "TPerfMon.h"
#include "TString.h"
#include "THttp.h"
#include <iostream>

Bool_t RootTest::TPerfMon::Update()
{
   // Retrieve the timings from the nightly build web page, and if the data is new,
   // parse and append to the history root file.

   THttpRequest req("http://www-root.fnal.gov/roottest/roottest_gmake.log.hennequeville");
   THttpReply* rep = req.Get();
   if (!rep || rep->IsZombie())
      return kFALSE;
   Parse(*rep);
   return kTRUE;
}

TCanvas* RootTest::TPerfMon::Draw()
{
   // Draw the tests' timing history, each normalized to 1 (for its average).

   return 0;
}


Bool_t RootTest::TPerfMon::Parse(const THttpReply& rep)
{
   TString line;
   Ssiz_t from = 0;
   while (rep.GetLine(line, from)) {
      if (!line.EndsWith("ms]")) continue;
      line.Remove(0, 11); // "Tests in ./"
      Ssiz_t posOK = line.Index("OK [");
      if (posOK == kNPOS) continue;
      TString testname = line(0, line.Index(" "));
      TString timing = line(posOK + 4, line.Length() - posOK - 3); // skip "OK [" and "ms]"
      fTiming[std::string(testname.Data())] = (Float_t) timing.Atof();
   }

   const THttpHeader* hdrLastMod = rep.GetHttpHeader("Last-Modified");
   if (hdrLastMod) fLastMod = hdrLastMod->GetString();
   std::cout << "Last modification: " << fLastMod << std::endl;
   for (std::map<std::string, float>::iterator iTiming = fTiming.begin();
      iTiming != fTiming.end(); ++iTiming)
      std::cout << iTiming->first << ": " << iTiming->second << std::endl;

   return kTRUE;
}

Bool_t RootTest::TPerfMon::UpdateTree(TTree* inTree)
{
   return kFALSE;
}
