#include "TString.h"
#include "TSystem.h"

void releaseCache(const char *set)
{

   TString cmd = TString::Format("%s/bin/releaseCache filelists/%s.txt", gSystem->Getenv("KSDIR"), set);

   gSystem->Exec(cmd);
}
