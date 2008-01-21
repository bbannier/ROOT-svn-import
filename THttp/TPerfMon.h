#ifndef INCLUDE_TPerfMon

#include "Rtypes.h"
#include <string>
#include <map>
#include "TString.h"

class TCanvas;
class TFile;
class TTree;
class THttpReply;

namespace RootTest {
   class TPerfMon {
   public:
      TPerfMon(const char* filename = "perfmonhist.root"):
         fFilename(filename), fHistFile(0) {}
      Bool_t Update();
      TCanvas* Draw();

   protected:
      Bool_t Parse(const THttpReply& rep);
      Bool_t UpdateTree(TTree* inTree);

   private:
      TString fFilename;
      TString fLastMod;
      TFile*   fHistFile;
      std::map<std::string /*test name*/, Float_t /* timing */> fTiming;
   };
}

#endif // INCLUDE_TPerfMon
