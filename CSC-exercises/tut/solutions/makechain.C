#include "TChain.h"
#include "TString.h"
TChain* makechain()
{
  TChain* ch = new TChain("MyTree");
  ch->Add("http://server/ROOT/exercise3.root");
  for (int i = 1; i < 6; ++i) {
    // Use TString::Format() to insert the file number (%d gets replaced by i)
    ch->Add(TString::Format("http://server/ROOT/exercise3_%d.root", i));
  }
  return ch;
}
